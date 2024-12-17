/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2017-2050 Huawei Technologies Co., Ltd.
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
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/ctype.h>
#include "huawei_pen_clk.h"
#include <huawei_thp_mt_wrapper.h>
#if ((defined CONFIG_FB) && (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK) && (!defined CONFIG_HUAWEI_THP_UDP) && (!defined CONFIG_DKMD_ENABLE))
#include "hisi_fb.h"
#endif
#ifdef CONFIG_HUAWEI_SENSORS_2_0
#include "sensor_scp.h"
#endif

#define SYSFS_PROPERTY_PATH "afe_properties"
#define SYSFS_TOUCH_PATH "touchscreen"
#define SYSFS_PLAT_TOUCH_PATH "huawei_touch"
#define THP_CHARGER_BUF_LEN 32
#define THP_BASE_DECIMAL 10
#define NEED_SCREEN_OFF 1
#define STYLUS3_CONNECT 2
#define TESTMODE_IN 1
#define TESTMODE_OUT 2
#define MOVE_8BIT 8
#define MAX_STATISTICS_TIME 255
#define MULTI_PANEL_NUM 2

u8 g_thp_log_cfg;
EXPORT_SYMBOL(g_thp_log_cfg);
static bool first_screen_switch;
static u32 g_active_panel;

u32 get_active_panel(void)
{
	return g_active_panel;
}

static u32 thp_get_panel_id_from_dev(struct device *dev)
{
	thp_log_debug("%s:dev name %s\n", __func__, dev->kobj.name);
	if (!strcmp(dev->kobj.name, "huawei_thp0"))
		return 0;
	if (!strcmp(dev->kobj.name, "huawei_thp1"))
		return 1;
	return 0;
}

static ssize_t thp_tui_wake_up_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);;

	ret = strncmp(buf, "open", sizeof("open"));
	if (ret == 0) {
		cd->thp_ta_waitq_flag = WAITQ_WAKEUP;
		wake_up_interruptible(&(cd->thp_ta_waitq));
		thp_log_err("%s wake up thp_ta_flag\n", __func__);
	}
	return count;
}

static ssize_t thp_tui_wake_up_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t thp_hostprocessing_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "hostprocessing\n");
}

static ssize_t thp_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	return snprintf(buf, PAGE_SIZE - 1, "status=0x%x\n",
		thp_get_status_all(panel_id));
}

/* If not config ic_name in dts, it will be "unknown" */
static ssize_t thp_chip_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (cd->hide_product_info_en && (!thp_is_factory()))
		return snprintf(buf, PAGE_SIZE, "%s\n", cd->project_id);

	return snprintf(buf, PAGE_SIZE, "%s-%s-%s\n",
		cd->thp_dev->ic_name,
		cd->project_id,
		cd->vendor_name ? cd->vendor_name : "unknown");
}

static ssize_t thp_loglevel_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 new_level = g_thp_log_cfg ? 0 : 1;

	int len = snprintf(buf, PAGE_SIZE, "%d -> %d\n",
		g_thp_log_cfg, new_level);

	g_thp_log_cfg = new_level;

	return len;
}

#if defined(THP_CHARGER_FB)
static ssize_t thp_host_charger_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	thp_log_debug("%s called\n", __func__);

	return snprintf(buf, THP_CHARGER_BUF_LEN, "%d\n",
			thp_get_status(THP_STATUS_CHARGER, panel_id));
}

static ssize_t thp_host_charger_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	/*
	 * get value of charger status from first byte of buf
	 */
	unsigned int value = buf[0] - '0';
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	thp_log_info("%s: input value is %d\n", __func__, value);

	thp_set_status(THP_STATUS_CHARGER, value, panel_id);

	return count;
}
#endif

static ssize_t thp_roi_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	short *roi_data = NULL;

	roi_data = cd->roi_data;
	memcpy(buf, roi_data, ROI_DATA_LENGTH * sizeof(short));

	return ROI_DATA_LENGTH * sizeof(short);
}

static const char *move_to_next_number(const char *str_in)
{
	const char *str = str_in;
	const char *next_num = NULL;

	str = skip_spaces(str);
	next_num = strchr(str, ' ');
	if (next_num)
		return next_num;

	return str_in;
}

static ssize_t thp_roi_data_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int i = 0;
	int num;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	short *roi_data = cd->roi_data;
	const char *str = buf;

	while (i < ROI_DATA_LENGTH && *str) {
		if (sscanf(str, "%7d", &num) < 0)
			break;

		str = move_to_next_number(str);
		roi_data[i++] = (short)num;
	}
	if (cd->need_notify_to_roi_algo)
		sysfs_notify(&dev->kobj, NULL, "roi_data");
	return count;
}

static ssize_t thp_roi_data_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int count = 0;
	int i;
	int ret;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	short *roi_data = cd->roi_data;

	for (i = 0; i < ROI_DATA_LENGTH; ++i) {
		ret = snprintf(buf + count, PAGE_SIZE - count, "%4d ",
			roi_data[i]);
		if (ret > 0)
			count += ret;
		/* every 7 data is a row */
		if (!((i + 1) % 7)) {
			ret = snprintf(buf + count, PAGE_SIZE - count, "\n");
			if (ret > 0)
				count += ret;
		}
	}

	return count;
}

static ssize_t thp_roi_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long status = 0;
	int ret;
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	ret = kstrtoul(buf, THP_BASE_DECIMAL, &status);
	if (ret) {
		thp_log_err("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_ROI, !!status, panel_id);
	thp_log_info("%s: set roi enable status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_roi_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
		thp_get_status(THP_STATUS_ROI, panel_id));
}

static ssize_t thp_holster_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long status = 0;
	int ret;
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	ret = kstrtoul(buf, THP_BASE_DECIMAL, &status);
	if (ret) {
		thp_log_err("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_HOLSTER, !!status, panel_id);
	thp_log_info("%s: set holster status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_holster_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
				thp_get_status(THP_STATUS_HOLSTER, panel_id));
}

static ssize_t thp_glove_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long status = 0;
	int ret;
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	ret = kstrtoul(buf, THP_BASE_DECIMAL, &status);
	if (ret) {
		thp_log_err("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_GLOVE, !!status, panel_id);
	thp_log_info("%s: set glove status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_glove_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
			thp_get_status(THP_STATUS_GLOVE, panel_id));
}


static ssize_t thp_holster_window_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int window_enable;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	ret = sscanf(buf, "%4d %4d %4d %4d %4d",
		&window_enable, &x0, &y0, &x1, &y1);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EBUSY;
	}
	cd->window.x0 = x0;
	cd->window.y0 = y0;
	cd->window.x1 = x1;
	cd->window.y1 = y1;

	thp_set_status(THP_STATUS_HOLSTER, !!window_enable, panel_id);
	thp_set_status(THP_STATUS_WINDOW_UPDATE,
		!thp_get_status(THP_STATUS_WINDOW_UPDATE, panel_id), panel_id);
	thp_log_info("%s: update window %d %d %d %d %d\n",
		__func__, window_enable, x0, y0, x1, y1);

	return count;
}

static ssize_t thp_holster_window_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_window_info *window = &cd->window;

	return snprintf(buf, PAGE_SIZE - 1, "%d %d %d %d %d\n",
			thp_get_status(THP_STATUS_HOLSTER, panel_id),
			window->x0, window->y0, window->x1, window->y1);
}

static ssize_t thp_touch_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int type = 0;
	int status = 0;
	int parameter = 0;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	ret = sscanf(buf, "%4d,%4d,%4d", &type, &status, &parameter);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EBUSY;
	}
	cd->scene_info.type = type;
	cd->scene_info.status = status;
	cd->scene_info.parameter = parameter;
	if (cd->support_interval_adjustment) {
		/* type:5 status:3 paramerter:0 means game mode on */
		if ((type == GAME_MODE_TYPE) &&
			(status == GAME_MODE_STATUS_ON) &&
			(parameter == GAME_MODE_PARAM))
			cd->time_adjustment_switch = true;
		/* type:5 status:4 paramerter:0 means game mode off */
		else if ((type == GAME_MODE_TYPE) &&
			(status == GAME_MODE_STATUS_OFF) &&
			(parameter == GAME_MODE_PARAM))
			cd->time_adjustment_switch = false;
		thp_log_info("%s: time_adjustment_switch = %d\n", __func__,
			cd->time_adjustment_switch);
	}

	thp_set_status(THP_STATUS_TOUCH_SCENE,
		!thp_get_status(THP_STATUS_TOUCH_SCENE, panel_id), panel_id);
	thp_log_info("%s:touch scene update %d %d %d\n",
		__func__, type, status, parameter);

	return count;
}

static ssize_t thp_touch_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int value = 0;

	thp_log_info("%s:value = %d\n", __func__, value);

	/*
	 * Inherit from tskit
	 * which will be use for ApsService to decide doze is support or not
	 * This feature is not supported by default in THP
	 */
	return snprintf(buf, PAGE_SIZE - 1, "%d\n", value);
}

static ssize_t thp_udfp_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	return snprintf(buf, PAGE_SIZE - 1, "udfp status : %d\n",
		thp_get_status(THP_STATUS_UDFP, panel_id));
}

static ssize_t thp_udfp_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int value = 0;
	int ret;
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	ret = sscanf(buf, "%d", &value);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EINVAL;
	}

	thp_set_status(THP_STATUS_UDFP, value, panel_id);
	thp_log_info("%s: ud fp status: %d\n", __func__, !!value);
#ifdef CONFIG_HUAWEI_SENSORS_2_0
	/* Use in-screen fingerprints on the mtk-53rd and 73rd platforms */
	send_sensor_scp_udfp(value);
#endif

	return count;
}

static ssize_t stylus_wakeup_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int stylus_mode = STYLUS_WAKEUP_DISABLE;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (cd->pen_supported == 0) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	return snprintf(buf, PAGE_SIZE - 1, "%d\n", stylus_mode);
}

static ssize_t send_stylus_status_in_mmi_test(unsigned int cur_status,
	unsigned int mode, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd) {
		thp_log_err("cd is null\n");
		return -EINVAL;
	}

	if ((cur_status & STYLUS3_CONNECT) != STYLUS3_CONNECT) {
		thp_log_info("ready set stylus3 status\n");

		if (mode == TESTMODE_IN)
			atomic_set(&cd->last_stylus3_status, STYLUS3_CONNECT);
		else if (mode == TESTMODE_OUT)
			atomic_set(&cd->last_stylus3_status, 0);

		if (cd->thp_dev->ops->bt_handler)
			if (cd->thp_dev->ops->bt_handler(cd->thp_dev, false))
				thp_log_err("ioctl mmi in send bt status fail\n");

		atomic_set(&cd->last_stylus3_status, cur_status);
	}

	return 0;
}

static ssize_t stylus_wakeup_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int parameter = 0;
	int ret;
	unsigned int cur_status;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (cd->send_bt_status_to_fw) {
		cur_status = atomic_read(&cd->last_stylus3_status);
		thp_log_info("%s:cur_status:%d\n", __func__, cur_status);
	}
	if (cd->pen_supported == 0) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	ret = sscanf(buf, "%1d", &parameter);
	if (ret <= 0)
		thp_log_err("%s:read node error\n", __func__);
	thp_log_info("%s:parameter:%d\n", __func__, parameter);
	switch (parameter) {
	case STYLUS_WAKEUP_TESTMODE_IN:
		thp_log_info("%s:enter test mode\n", __func__);
		if (cd->send_bt_status_to_fw) {
			cd->enter_stylus3_mmi_test = true;
			send_stylus_status_in_mmi_test(cur_status, TESTMODE_IN, panel_id);
		}
		cd->stylus_status = thp_get_status(THP_STATUS_STYLUS, panel_id);
		cd->stylus3_status = thp_get_status(THP_STATUS_STYLUS3, panel_id);
		/* set bit 1 to turn on the pen switch */
		thp_set_status(THP_STATUS_STYLUS, 1, panel_id);
		thp_set_status(THP_STATUS_STYLUS3, 1, panel_id);
		break;
	case STYLUS_WAKEUP_TESTMODE_OUT:
		thp_log_info("%s:exit test mode, recover last status\n",
			__func__);
		if (cd->send_bt_status_to_fw) {
			cd->enter_stylus3_mmi_test = false;
			send_stylus_status_in_mmi_test(cur_status, TESTMODE_OUT, panel_id);
		}
		thp_set_status(THP_STATUS_STYLUS, cd->stylus_status, panel_id);
		thp_set_status(THP_STATUS_STYLUS3, cd->stylus3_status, panel_id);
		break;
	case STYLUS_WAKEUP_DISABLE:
		/* set bit 0 to turn off the pen switch */
		thp_set_status(THP_STATUS_STYLUS, 0, panel_id);
		cd->stylus_status = parameter;
		break;
	case STYLUS_WAKEUP_NORMAL_STATUS:
	case STYLUS_WAKEUP_LOW_FREQENCY:
		if (cd->unsupported_stylus == 0) {
			/* set bit 1 to turn on the pen switch */
			thp_set_status(THP_STATUS_STYLUS, 1, panel_id);
			cd->stylus_status = parameter;
		}
		break;
	default:
		thp_log_info("%s:invalid  input\n", __func__);
		break;
	}
	return count;
}

static ssize_t thp_supported_func_indicater_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
		cd->supported_func_indicater);
}

static ssize_t thp_easy_wakeup_gesture_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_easy_wakeup_info *info = &cd->easy_wakeup_info;
	ssize_t ret;

	if (!cd->support_gesture_mode) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s\n", __func__);

	ret = snprintf(buf, MAX_STR_LEN, "0x%04X\n",
		info->easy_wakeup_gesture);

	return ret;
}

static ssize_t thp_easy_wakeup_gesture_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_easy_wakeup_info *info = NULL;
	unsigned int value = 0;
	int ret;

	if (!cd->support_gesture_mode) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s: called\n", __func__);

	ret = sscanf(buf, "%5u", &value);
	if ((ret <= 0) || (value > TS_GESTURE_INVALID_COMMAND)) {
		thp_log_err("invalid parm\n");
		return -EBUSY;
	}
	info = &cd->easy_wakeup_info;
	info->easy_wakeup_gesture = value;
	cd->double_click_switch = (is_app_enable_gesture(GESTURE_DOUBLE_CLICK) &
		info->easy_wakeup_gesture) ? DOUBLE_CLICK_ON : DOUBLE_CLICK_OFF;
	cd->single_click_switch = (is_app_enable_gesture(GESTURE_SINGLE_CLICK) &
		info->easy_wakeup_gesture) ? SINGLE_CLICK_ON : SINGLE_CLICK_OFF;
	thp_log_info("easy_wakeup_gesture=0x%x\n", info->easy_wakeup_gesture);
	thp_log_info("double_click_switch:%u, single_click_switch %u\n",
		cd->double_click_switch, cd->single_click_switch);
	if (info->easy_wakeup_gesture == false) {
		cd->sleep_mode = TS_POWER_OFF_MODE;
		thp_log_info("poweroff mode\n");
	} else {
		cd->sleep_mode = TS_GESTURE_MODE;
		thp_log_info("gesture mode\n");
	}

	thp_log_info("ts gesture wakeup done\n");
	return size;
}

static ssize_t thp_wakeup_gesture_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long tmp = 0;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int error;

	if (!cd->support_gesture_mode) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s called\n", __func__);

	error = sscanf(buf, "%4lu", &tmp);
	if (error <= 0) {
		thp_log_err("sscanf return invaild :%d\n", error);
		return -EINVAL;
	}
	thp_log_info("%s:%lu\n", __func__, tmp);
	return count;
}

static ssize_t thp_wakeup_gesture_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd->support_gesture_mode) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s called\n", __func__);
	return 0;
}

static ssize_t thp_easy_wakeup_control_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned long value = 0;
	int ret;
	int error;

	if (!cd->support_gesture_mode) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s called\n", __func__);

	ret = sscanf(buf, "%4lu", &value);
	if (ret <= 0 || value > TS_GESTURE_INVALID_CONTROL_NO) {
		thp_log_info("%s->invalid parm\n", __func__);
		return -EINVAL;
	}

	value = (u8) value & TS_GESTURE_COMMAND;
	if (value == 1) {
		if (cd->thp_dev->ops->chip_wrong_touch) {
			error = cd->thp_dev->ops->chip_wrong_touch(cd->thp_dev);
			if (error < 0)
				thp_log_info("chip_wrong_touch error\n");
		} else {
			thp_log_info("chip_wrong_touch not init\n");
		}
	}
	thp_log_info("%s done\n", __func__);
	return size;
}

static ssize_t thp_easy_wakeup_position_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd->support_gesture_mode) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static ssize_t thp_oem_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error = NO_ERR;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	thp_log_info("%s: called\n", __func__);

	if (cd->support_oem_info == THP_OEM_INFO_LCD_EFFECT_TYPE) {
		error = snprintf(buf, OEM_INFO_DATA_LENGTH, "%s",
			cd->oem_info_data);
		if (error < 0) {
			thp_log_info("%s:oem info data:%s\n",
				__func__, cd->oem_info_data);
			return error;
		}
		thp_log_info("%s: oem info :%s\n", __func__, buf);
	}

	thp_log_debug("%s done\n", __func__);
	return error;
}


static ssize_t thp_fingerprint_switch_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned int udfp_enable = 0;
	unsigned int udfp_switch = 0;
	int ret;

	if (!cd->support_fingerprint_switch) {
		thp_log_debug("%s:no need to report fingerprint switch\n",
			__func__);
		return count;
	}
	/*
	 * 0,0:the fingerprint doesn't be enrolled
	 * 1,1:begin enroll or authenticate
	 * 1,0:end enroll or authenticate
	 */
	ret = sscanf(buf, "%4u,%4u", &udfp_enable, &udfp_switch);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EINVAL;
	}

	cd->udfp_status.udfp_enable = udfp_enable;
	cd->udfp_status.udfp_switch = udfp_switch;

	thp_set_status(THP_STATUS_UDFP_SWITCH, !!udfp_switch, panel_id);
	thp_log_info("%s: update udfp_status:%u %u\n",
		__func__, udfp_enable, udfp_switch);

	return count;
}

static ssize_t thp_fingerprint_switch_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	return snprintf(buf, PAGE_SIZE - 1, "%u,%u\n",
		cd->udfp_status.udfp_enable, cd->udfp_status.udfp_switch);
}

static ssize_t thp_ring_switch_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned int ring_status = 0;
	int ret;

	ret = sscanf(buf, "%4u", &ring_status);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s:ring_status:%u\n", __func__, ring_status);
	if (!cd->support_ring_feature) {
		thp_log_err("%s: not support ring\n", __func__);
		return -EINVAL;
	}
	cd->ring_switch = RING_SWITCH_OFF;
	cd->phone_status = PNONE_STATUS_OFF;
	/*
	 * ring_status = 0:disbale report ring event
	 * ring_status = 1:enable report ring event
	 * ring_status = 2:on the phone
	 */
	switch (ring_status) {
	case RING_STAUS_ENABLE:
		cd->ring_switch = RING_SWITCH_ON;
		break;
	case RING_PHONE_STATUS:
		cd->ring_switch = RING_SWITCH_ON;
		cd->phone_status = PNONE_STATUS_ON;
		break;
	default:
		break;
	}

#ifdef CONFIG_HUAWEI_SHB_THP
	if ((cd->suspended || cd->support_ring_feature) &&
		(ring_status <= RING_PHONE_STATUS)) {
		ret = send_thp_driver_status_cmd(ring_status,
			0, ST_CMD_TYPE_SET_AUDIO_STATUS);
		thp_log_info("%s: %d\n", __func__, ret);
	}
#endif
	return count;
}

static ssize_t thp_ring_switch_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	return snprintf(buf, PAGE_SIZE - 1, "%u\n", cd->ring_switch);
}

static ssize_t thp_power_switch_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned int power_status = 0;
	int ret;

	ret = sscanf(buf, "%u", &power_status);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s:power_status:0x%x\n", __func__, power_status);
	switch (power_status) {
	case WAKE_REASON_POWER_BUTTON:
		cd->power_switch = POWER_KEY_ON_CTRL;
		break;
	case WAKE_REASON_FINGER_PRINT:
	case WAKE_REASON_PICKUP:
		cd->power_switch = FINGERPRINT_ON_CTRL;
		break;
	case WAKE_REASON_PROXIMITY:
		cd->power_switch = PROXIMITY_ON_CTRL;
		break;
	case WAKE_REASON_UNKNOWN:
	case WAKE_REASON_APPLICATION:
	case WAKE_REASON_PLUGGED_IN:
	case WAKE_REASON_GESTURE:
	case WAKE_REASON_CAMERA_LAUNCH:
	case WAKE_REASON_WAKE_KEY:
	case WAKE_REASON_WAKE_MOTION:
	case WAKE_REASON_HDMI:
	case WAKE_REASON_LID:
		cd->power_switch = AUTO_OR_OTHER_ON_CTRL;
		break;
	case GO_TO_SLEEP_REASON_POWER_BUTTON:
		cd->power_switch = POWER_KEY_OFF_CTRL;
		break;
	case GO_TO_SLEEP_REASON_PROX_SENSOR:
		cd->power_switch = PROXIMITY_OFF_CTRL;
		break;
	case GO_TO_SLEEP_REASON_TIMEOUT:
		cd->power_switch = POWERON_TIMEOUT_OFF_CTRL;
		break;
	case GO_TO_SLEEP_REASON_APPLICATION:
	case GO_TO_SLEEP_REASON_DEVICE_ADMIN:
	case GO_TO_SLEEP_REASON_LID_SWITCH:
	case GO_TO_SLEEP_REASON_HDMI:
	case GO_TO_SLEEP_REASON_ACCESSIBILITY:
	case GO_TO_SLEEP_REASON_FORCE_SUSPEND:
	case GO_TO_SLEEP_REASON_WAIT_BRIGHTNESS_TIMEOUT:
	case GO_TO_SLEEP_REASON_PHONE_CALL:
		cd->power_switch = AUTO_OR_OTHER_OFF_CTRL;
		break;
	default:
		cd->power_switch = POWER_STATUS_NULL;
		return -EINVAL;
	}
	thp_log_info("%s :power_switch = %u\n", __func__, cd->power_switch);
	thp_set_status(THP_STATUS_POWER_SWITCH_CTRL,
		!thp_get_status(THP_STATUS_POWER_SWITCH_CTRL, panel_id), panel_id);
	return count;
}

static ssize_t thp_power_switch_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	return snprintf(buf, PAGE_SIZE - 1, "%u\n", cd->power_switch);
}

#define MAX_SURPPORT_SPEED 30000000
#define MIN_SURPPORT_SPEED 1000000
#define SET_SPI_SPEED_MODE 1
static int thp_set_spi_speed_store(int data, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int spi_speed = data;
	int rc = 0;

	if (thp_is_factory()) {
		if ((spi_speed > MAX_SURPPORT_SPEED) ||
			(spi_speed < MIN_SURPPORT_SPEED)) {
			thp_log_err("%s:invalid spi speed = %d\n", __func__, spi_speed);
			return -EINVAL;
		}
		cd->spi_speed_mode = SET_SPI_SPEED_MODE;
		rc = thp_set_spi_max_speed(spi_speed, cd->panel_id);
		if (rc) {
			thp_log_err("%s: failed to set spi  speed\n", __func__);
			cd->spi_speed_mode = 0;
		}
		thp_log_info("%s:spi_speed mode: %u, spi_speed: %d\n",
			__func__, cd->spi_speed_mode, spi_speed);
	}
	return rc;
}

static int thp_screen_off_cap_test_control(int data, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if ((cd->support_factory_mode_extra_cmd != RT_CONTROL_LCD_OFF) ||
		(!thp_is_factory())) {
		thp_log_info("%s:not support RT control LCD power off\n",
			__func__);
		return -EINVAL;
	}
	cd->always_poweron_in_screenoff = data;
	thp_log_info("%s always_poweron_in_screenoff = %d\n",
		__func__, cd->always_poweron_in_screenoff);
	return 0;
}

static ssize_t thp_factory_extra_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned int type = 0;
	int data = 0;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
#if ((defined CONFIG_FB) && (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK) && (!defined CONFIG_HUAWEI_THP_UDP) && (!defined CONFIG_DKMD_ENABLE))
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];
#endif

	if ((cd->support_factory_mode_extra_cmd == 0) || (cd->suspended)) {
		thp_log_err("%s: not support or suspended\n", __func__);
		return -EINVAL;
	}
	ret = sscanf(buf, "%3u,%4d", &type, &data);
	if (ret <= 0) {
		thp_log_info("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s: type:%u, data:%d\n",
		__func__, type, data);
	switch (type) {
	case SUSPEND_NEED_CAP_TEST:
		if (thp_is_factory()) {
			thp_log_info("%s in factory mode\n", __func__);
#if ((defined CONFIG_FB) && (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK) && (!defined CONFIG_HUAWEI_THP_UDP) && (!defined CONFIG_DKMD_ENABLE))
			if (data == NEED_SCREEN_OFF) {
				cd->always_poweron_in_screenoff = data;
				cd->factory_mode_current_backlight =
					hisifd->bl_level;
				thp_log_info("%s lcd power off current_bl = %d\n",
					__func__,
					cd->factory_mode_current_backlight);
				hisi_fb_blank_sub(FB_BLANK_POWERDOWN,
					hisifd->fbi);
			} else {
				hisi_fb_blank_sub(FB_BLANK_UNBLANK,
					hisifd->fbi);
				/* triggers refresh and brightness setting */
				hisi_fb_frame_refresh(hisifd, "esd");
				msleep(100);
				hisifb_set_backlight(hisifd,
					cd->factory_mode_current_backlight,
					false);
				thp_log_info("%s lcd power on current_bl = %d\n",
					__func__,
					cd->factory_mode_current_backlight);
				cd->always_poweron_in_screenoff = data;
			}
#endif
		}
		break;
	case RT_CONTROL_LCD_OFF:
		if (thp_screen_off_cap_test_control(data, panel_id))
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}
	return count;
}

#define POLYMERIC_CHIP_STATUS_REG 0x25f0054
#define MAX_CHIP_ID 2
void thp_get_polymeric_chip_status(u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);
#ifdef CONFIG_CONTEXTHUB_IO_DIE_STS
	int chip_id;
	int ret;
	sts_reg_op_t data;

	if (!cd->support_polymeric_chip)
		return;
	data.reg_addr = POLYMERIC_CHIP_STATUS_REG;
	data.op = STS_SUB_CMD_REG_READ;
	for (chip_id = 0; chip_id < MAX_CHIP_ID; chip_id++) {
		data.slave_id = chip_id;
		data.val = 0;
		ret = sts_reg_op(&data);
		thp_log_info("%s:chip_id %d, ret = %d, value = %u\n",
			__func__, chip_id, ret, data.val);
	}
#else
	(void)cd;
#endif
}

static ssize_t thp_dfx_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned int type = 0;
	int data = 0;
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	ret = sscanf(buf, "%3u,%10d", &type, &data);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s: type:%u, data:%d\n",
		__func__, type, data);
	switch (type) {
	case POLYMERIC_CHIP_INFO:
		thp_get_polymeric_chip_status(panel_id);
		break;
	case SET_SPI_SPEED:
		ret = thp_set_spi_speed_store(data, panel_id);
		if (ret)
			return -EINVAL;
		break;
	case GET_IRQ_INFO:
		thp_get_irq_info(panel_id);
		break;
	default:
		return -EINVAL;
	}
	return count;
}

static ssize_t aod_touch_switch_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned int aod_touch_enable;
	int ret;

	if (!cd->aod_display_support) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}

	ret = sscanf(buf, "%4u", &aod_touch_enable);
	if (ret <= 0) {
		thp_log_err("%s: illegal input, ret = %d\n", __func__, ret);
		return -EINVAL;
	}

	cd->aod_touch_status = aod_touch_enable;
	thp_log_info("%s: update aod_touch_enable: %u\n", __func__,
		aod_touch_enable);
	return count;
}

static ssize_t aod_touch_switch_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd->aod_display_support) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	return snprintf(buf, PAGE_SIZE - 1, "%u\n", cd->aod_touch_status);
}

static ssize_t thp_ring_setting_config_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int ret;
	unsigned int type = 0;
	unsigned int data = 0;
	unsigned int volume_side;

	if (cd->support_ring_feature == 0) {
		thp_log_err("%s: not support\n", __func__);
		return -EINVAL;
	}
	volume_side = cd->volume_side_status;
	ret = sscanf(buf, "%u,%u", &type, &data);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s: type:%u, data:%u\n",
		__func__, type, data);
	switch (type) {
	case RING_FEATURE_MODE:
		cd->volume_side_status = data & VOLUME_SIDE_STATUS_BOTH;
		if (cd->support_ring_setting_switch)
			cd->ring_setting_switch = !!data;
		if (cd->volume_side_status != volume_side)
			thp_set_status(THP_STATUS_VOLUME_SIDE,
				!thp_get_status(THP_STATUS_VOLUME_SIDE, panel_id), panel_id);
		break;
	default:
		return -EINVAL;
	}
	return count;
}

static ssize_t thp_ring_setting_config_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	return snprintf(buf, PAGE_SIZE - 1, "%u\n",
		cd->volume_side_status);
}

bool is_first_screen_switch(void)
{
	return first_screen_switch;
}

static bool check_thp_dev(u32 panel_id)
{
	bool flag = true;
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	u32 i;

	for (i = 0; i <= cd->dual_thp; i++)
		flag = flag && (!!thp_get_core_data(i)->thp_dev);
	return flag;
}

static ssize_t thp_screen_status_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "%u\n",
		g_active_panel);
}

ssize_t thp_screen_status_switch(struct thp_core_data *cd,
	u32 screen_status, size_t count)
{
	static u32 last_screen_status = SCREEN_INVAILD_STATUS;
	bool is_repeat_status = false;
	struct thp_cmd_node cmd;
#ifndef CONFIG_MODULE_KO_TP
#ifdef CONFIG_LCD_KIT_DRIVER
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
#endif
#endif

	/*
	 * (last_screen_status != SCREEN_INVAILD_STATUS) means not fold or unfold firstly
	 * ((last_screen_status > 0) && (screen_status > 0)) means fold to fold
	 * ((last_screen_status == 0) && (screen_status == 0)) means unfold to unfold
	 */
	is_repeat_status = (last_screen_status != SCREEN_INVAILD_STATUS) &&
		(((last_screen_status > 0) && (screen_status > 0)) ||
		((last_screen_status == 0) && (screen_status == 0)));
	if (is_repeat_status) {
		thp_log_err("%s:repeat status:0x%x, last status:0x%x\n", __func__,
			screen_status, last_screen_status);
		return count;
	}
	last_screen_status = screen_status;

	memset(&cmd, 0, sizeof(cmd));
	if (screen_status == SCREEN_FOLDED) {
		cmd.command = TS_SCREEN_FOLD;
		g_active_panel = SUB_TOUCH_PANEL;
	} else if (screen_status == SCREEN_OFF_FOLD) {
		cmd.command = TS_SCREEN_OFF_FOLD;
		g_active_panel = SUB_TOUCH_PANEL;
	} else if (screen_status == SCREEN_UNFOLD) {
		cmd.command = TS_SCREEN_UNFOLD;
		g_active_panel = MAIN_TOUCH_PANEL;
	} else {
		goto err;
	}
#ifndef CONFIG_MODULE_KO_TP
#ifdef CONFIG_LCD_KIT_DRIVER
	if (tp_ops && tp_ops->notify_panel_switch)
		tp_ops->notify_panel_switch();
#endif
#endif
	cmd.cmd_param.pub_params.dev = cd->thp_dev;
	if (cd->dual_thp)
		cmd.cmd_param.panel_id = g_active_panel;
	else
		cmd.cmd_param.panel_id = cd->panel_id;
	cmd.cmd_param.thread_id = get_available_thread_id();
	if (thp_put_one_cmd(&cmd, 0))
		thp_log_err("%s: put cmd error\n", __func__);

	return count;
err:
	thp_log_err("%s:invalid status: 0x%x\n", __func__, screen_status);
	return -EINVAL;
}

static ssize_t thp_screen_status_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	u32 screen_status = SCREEN_UNFOLD;
	int ret;

	if (!cd->support_screen_switch) {
		thp_log_err("%s: not support screen switch\n", __func__);
		return -EINVAL;
	}
	if (!first_screen_switch)
		first_screen_switch = true;
	if (!check_thp_dev(panel_id)) {
		thp_log_err("%s: exist null thp_dev\n", __func__);
		return -EINVAL;
	}

	ret = sscanf(buf, "%u", &screen_status);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s:screen_status:0x%x\n", __func__, screen_status);
	thp_power_lock();
	count = thp_screen_status_switch(cd, screen_status, count);
	thp_power_unlock();
	return count;
}

#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
#define AOD_WINDOW_READY_STATUS 2
static ssize_t aod_event_report_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	struct thp_aod_window window;
	unsigned int status = 0;
	int ret;

	if (!cd->use_ap_gesture) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	ret = sscanf(buf, "%4u %4u %4u %4u %4u", &status, &window.x0,
		&window.y0, &window.x1, &window.y1);
	if (ret <= 0) {
		thp_log_err("%s: illegal input, ret = %d\n", __func__, ret);
		return -EINVAL;
	}
	thp_log_info("%s:status: %u, x0:%u, y0:%u, x1:%u, y1:%u\n",
		__func__, status, window.x0, window.y0, window.x1, window.y1);

	if (status == AOD_WINDOW_READY_STATUS) {
		cd->aod_window_ready_status = true;
		return count;
	}

	cd->aod_event_report_status = status;
	if (!cd->thp_dev->ops->tp_aod_event_ctrl) {
		thp_log_err("%s: ops is NULL\n", __func__);
		return -EINVAL;
	}
	ret = cd->thp_dev->ops->tp_aod_event_ctrl(cd->thp_dev,
		cd->aod_event_report_status, window);
	if (ret < 0) {
		thp_log_err("%s: aod event ctrl error, %d\n", __func__, ret);
		return -EINVAL;
	}
	return count;
}

static ssize_t tp_ud_lowpower_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned int status = 0;
	int ret;

	if (!cd->use_ap_gesture) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	ret = sscanf(buf, "%1u", &status);
	if (ret <= 0) {
		thp_log_err("%s: illegal input, ret = %d\n", __func__, ret);
		return -EINVAL;
	}
	cd->tp_ud_lowpower_status = status;
	thp_log_info("%s: update lowpower_status: %u\n", __func__, status);
	if (!cd->thp_dev->ops->tp_lowpower_ctrl) {
		thp_log_err("%s: ops is NULL\n", __func__);
		return -EINVAL;
	}
	ret = cd->thp_dev->ops->tp_lowpower_ctrl(cd->thp_dev, status);
	if (ret < 0) {
		thp_log_err("%s: lowpower ctrl error, %d\n", __func__, ret);
		return -EINVAL;
	}
	return count;
}

static unsigned int check_panel_from_aod(unsigned int panel,
	unsigned int need_check_panel_from_aod)
{
	if (need_check_panel_from_aod && (panel != g_active_panel)) {
		thp_log_err("%s: panel from aod not match active panel, use active panel %u\n",
			__func__, g_active_panel);
		return g_active_panel;
	}
	return panel;
}

static ssize_t aod_notify_for_multi_panel(const char *buf, size_t count, u32 panel_id)
{
	int ret;
	unsigned int panel;
	int status;
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned int dual_thp = cd->dual_thp;

	ret = sscanf(buf, "%1u,%1u", &panel, &status);
	if (ret <= 0) {
		thp_log_err("%s: illegal input, ret = %d\n", __func__, ret);
		return -EINVAL;
	}
	thp_log_info("%s: panel is %u, status is %u\n", __func__, panel, status);
	panel = check_panel_from_aod(panel, cd->need_check_panel_from_aod);
	if (dual_thp)
		cd = thp_get_core_data(panel);
	thp_power_lock();
	mutex_lock(&cd->aod_power_ctrl_mutex);
	switch (panel) {
	case 0: /* 0 is main panel */
		cd->aod_notify_tp_power_status = status;
		aod_status_notifier(cd, status);
		break;
	case 1: /* 1 is sub panel */
		cd->aod_notify_tp_power_status = status;
		if (!dual_thp)
			tskit_aod_status_notifier(status);
		else
			aod_status_notifier(cd, status);
		break;
	default:
		break;
	}
	mutex_unlock(&cd->aod_power_ctrl_mutex);
	thp_power_unlock();
	return count;
}

static ssize_t aod_notify_tp_power_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	unsigned int status;
	int ret;

	if (!cd->use_aod_power_ctrl_notify) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		return aod_notify_for_multi_panel(buf, count, panel_id);
	thp_power_lock();
	mutex_lock(&cd->aod_power_ctrl_mutex);
	ret = sscanf(buf, "%1u", &status);
	if (ret <= 0) {
		thp_log_err("%s: illegal input, ret = %d\n", __func__, ret);
		return -EINVAL;
	}
	cd->aod_notify_tp_power_status = status;
	thp_log_info("%s: update aod_notify_tp_power_status: %u\n", __func__,
		status);
	aod_status_notifier(cd, status);
	mutex_unlock(&cd->aod_power_ctrl_mutex);
	thp_power_unlock();
	return count;
}

static ssize_t aod_notify_tp_power_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (!cd->use_aod_power_ctrl_notify) {
		thp_log_err("%s not support\n", __func__);
		return -EINVAL;
	}
	return snprintf(buf, PAGE_SIZE - 1, "%u\n",
		cd->aod_notify_tp_power_status);
}
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
static ssize_t jkm_notify_tp_power_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned int panel;
	unsigned int status;
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	ret = sscanf(buf, "%1u,%1u", &panel, &status);
	if (ret <= 0) {
		thp_log_err("%s: illegal input, ret = %d\n", __func__, ret);
		return -EINVAL;
	}
	thp_log_info("%s: panel is %u, status is %u\n", __func__, panel, status);
	if (cd->dual_thp)
		cd = thp_get_core_data(panel);
	mutex_lock(&cd->aod_power_ctrl_mutex);
	switch (panel) {
	case 0: /* 0 is main panel */
		aod_status_notifier(cd, status);
		break;
	case 1: /* 1 is sub panel */
		if (!cd->dual_thp)
			tskit_aod_status_notifier(status);
		else
			aod_status_notifier(cd, status);
		break;
	default:
		break;
	}
	mutex_unlock(&cd->aod_power_ctrl_mutex);
	return count;
}
#endif

static ssize_t thp_app_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	int ret;

	memset(cd->app_info, 0, MAX_APP_INFO_LEN);
	ret = snprintf(cd->app_info, MAX_APP_INFO_LEN, "%s", buf);
	if (ret <= 0) {
		thp_log_err("%s: input error, ret = %d\n", __func__, ret);
		return -EINVAL;
	}

	thp_log_info("%s:app_info: %s\n", __func__, cd->app_info);
	/*
	 * 1.when app frame rate change to 144HZ, algorithm can't reduce the stylus's noise.
	 * the stylus has ghost spots issue, in this case, the stylus's uplink need to close.
	 * 2.the info of app for enter/exit 144Hz frame rate will affect the algorithm of
	 * switchover finger reporting rate. so, these two app types can't send to daemon,
	 * need to be filtered.
	 */
	if (cd->app_send_stylus3_status_to_fw) {
		ret = app_send_stylus3_status_to_fw(cd);
		if (ret == 0)
			return count;
	}
	thp_set_status(THP_STATUS_APP_INFO, !thp_get_status(THP_STATUS_APP_INFO, panel_id), panel_id);
	return count;
}

static ssize_t thp_app_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	return snprintf(buf, MAX_APP_INFO_LEN, "%s\n", cd->app_info);
}

static void sleep_mode_pen_clk_control(int sleep_mode_status, u32 panel_id)
{
	struct thp_core_data *cd = thp_get_core_data(panel_id);

	if (panel_id != MAIN_TOUCH_PANEL)
		return;
	if (!cd->suspended) {
		thp_log_info("%s:resume state\n", __func__);
		return;
	}
	if (sleep_mode_status) {
		stop_pen_clk_timer();
		(void)thp_pen_clk_disable();
	} else {
		pen_clk_calibration();
	}
	thp_log_info("%s:out\n", __func__);
}

static ssize_t thp_phone_sleep_mode_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	u32 phone_sleep_mode_status = 0;
	u32 total_time = 0;
	u32 active_time = 0;
	u32 para;
	int ret;

	if (!cd->support_phone_sleep_mode) {
		thp_log_err("%s: not support\n", __func__);
		return -EINVAL;
	}

	ret = sscanf(buf, "%2u,%4u,%4u", &phone_sleep_mode_status,
		&total_time, &active_time);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s:phone_sleep_mode_status=%u,total_time=%u, active_time=%u\n",
		__func__, phone_sleep_mode_status, total_time, active_time);
	if (cd->need_extra_system_clk) {
		sleep_mode_pen_clk_control(!!phone_sleep_mode_status, panel_id);
		return count;
	}

	if ((total_time > MAX_STATISTICS_TIME) ||
		(active_time > MAX_STATISTICS_TIME))
		return -EINVAL;
	para = ((phone_sleep_mode_status & 0xFF) << MOVE_8BIT) + total_time;
	para = (para << MOVE_8BIT) + active_time;
#ifdef CONFIG_HUAWEI_SHB_THP
	ret = send_thp_driver_status_cmd(0, para,
		ST_CMD_TYPE_SET_PHONE_SLEEP_MODE_STATUS);
	thp_log_info("%s: %d\n", __func__, ret);
#endif
	return count;
}

static ssize_t thp_magnification_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int magnification;
	u32 panel_id = thp_get_panel_id_from_dev(dev);

	magnification = thp_get_finger_resolution_magnification(panel_id);
	return snprintf(buf, PAGE_SIZE - 1, "%u\n", magnification);
}

static ssize_t thp_daemon_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 panel_id = thp_get_panel_id_from_dev(dev);
	struct thp_core_data *cd = thp_get_core_data(panel_id);
	u32 d_version;

	d_version = cd->aptouch_daemon_version;
	thp_log_info("daemon_version: %u\n", d_version);
	return snprintf(buf, PAGE_SIZE - 1, "%u\n", d_version);
}

#ifdef CONFIG_HISYSEVENT
#define MAX_EVENT_LEN 32
static ssize_t thp_hisysevent_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	const char *event_name = NULL;

	if (count > MAX_EVENT_LEN) {
		thp_log_err("%s: illegal input,len is %d!\n", __func__, count);
		return count;
	}

	event_name = vzalloc(MAX_EVENT_LEN);
	if (!event_name) {
		thp_log_err("%s: alloc failed!\n", __func__);
		return count;
	}

	ret = sscanf(buf, "%s", event_name);
	if (ret <= 0) {
		thp_log_err("%s: illegal input\n", __func__);
		vfree(event_name);
		return -EINVAL;
	}

	thp_hiview_hisysevent(event_name, "test hisysevent: %s ", event_name);
	vfree(event_name);
	return count;
}
#endif

static DEVICE_ATTR(thp_status, 0444, thp_status_show, NULL);
static DEVICE_ATTR(touch_chip_info, 0640,
	thp_chip_info_show, NULL);
static DEVICE_ATTR(hostprocessing, 0444, thp_hostprocessing_show, NULL);
static DEVICE_ATTR(loglevel, 0444, thp_loglevel_show, NULL);
static DEVICE_ATTR(charger_state, 0660,
	thp_host_charger_state_show, thp_host_charger_state_store);
static DEVICE_ATTR(roi_data, 0660,
	thp_roi_data_show, thp_roi_data_store);
static DEVICE_ATTR(roi_data_internal, 0400, thp_roi_data_debug_show, NULL);
static DEVICE_ATTR(roi_enable, 0660,
	thp_roi_enable_show, thp_roi_enable_store);
static DEVICE_ATTR(touch_sensitivity, 0660,
	thp_holster_enable_show, thp_holster_enable_store);
static DEVICE_ATTR(touch_glove, 0660,
	thp_glove_enable_show, thp_glove_enable_store);
static DEVICE_ATTR(touch_window, 0660,
	thp_holster_window_show, thp_holster_window_store);
static DEVICE_ATTR(touch_switch, 0660,
	thp_touch_switch_show, thp_touch_switch_store);
static DEVICE_ATTR(udfp_enable, 0660,
	thp_udfp_enable_show, thp_udfp_enable_store);
static DEVICE_ATTR(stylus_wakeup_ctrl, 0660,
	stylus_wakeup_ctrl_show, stylus_wakeup_ctrl_store);
static DEVICE_ATTR(supported_func_indicater, 0400,
	thp_supported_func_indicater_show, NULL);
static DEVICE_ATTR(tui_wake_up_enable, 0660,
	thp_tui_wake_up_enable_show, thp_tui_wake_up_enable_store);
static DEVICE_ATTR(easy_wakeup_gesture, 0660,
	thp_easy_wakeup_gesture_show, thp_easy_wakeup_gesture_store);
static DEVICE_ATTR(wakeup_gesture_enable, 0600,
	thp_wakeup_gesture_enable_show, thp_wakeup_gesture_enable_store);
static DEVICE_ATTR(easy_wakeup_control, 0200, NULL,
	thp_easy_wakeup_control_store);
static DEVICE_ATTR(easy_wakeup_position, 0400, thp_easy_wakeup_position_show,
	NULL);
static DEVICE_ATTR(touch_oem_info, 0640,
	thp_oem_info_show, NULL);
static DEVICE_ATTR(fingerprint_switch_ctrl, 0660,
	thp_fingerprint_switch_ctrl_show, thp_fingerprint_switch_ctrl_store);
static DEVICE_ATTR(ring_switch_ctrl, 0660,
	thp_ring_switch_ctrl_show, thp_ring_switch_ctrl_store);
static DEVICE_ATTR(power_switch_ctrl, 0660,
	thp_power_switch_ctrl_show, thp_power_switch_ctrl_store);
static DEVICE_ATTR(tp_dfx_info, 0660, NULL, thp_dfx_info_store);
static DEVICE_ATTR(aod_touch_switch_ctrl, 0660,
	aod_touch_switch_ctrl_show, aod_touch_switch_ctrl_store);
static DEVICE_ATTR(ring_setting_config_ctrl, 0660,
	thp_ring_setting_config_ctrl_show, thp_ring_setting_config_ctrl_store);
static DEVICE_ATTR(factory_extra_cmd, 0660, NULL,
	thp_factory_extra_cmd_store);
static DEVICE_ATTR(screen_status_switch, 0660, thp_screen_status_switch_show,
	thp_screen_status_switch_store);
static DEVICE_ATTR(thp_app_info, 0660, thp_app_info_show,
	thp_app_info_store);
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
static DEVICE_ATTR(aod_event_report_ctrl, 0660, NULL,
	aod_event_report_ctrl_store);
static DEVICE_ATTR(tp_ud_lowpower_ctrl, 0660, NULL, tp_ud_lowpower_ctrl_store);
static DEVICE_ATTR(aod_notify_tp_power_ctrl, 0660,
	aod_notify_tp_power_status_show, aod_notify_tp_power_status_store);
#endif
#if defined(CONFIG_LCD_KIT_DRIVER)
static DEVICE_ATTR(jkm_notify_tp_power_ctrl, 0660, NULL,
	jkm_notify_tp_power_status_store);
#endif
static DEVICE_ATTR(phone_sleep_mode_status, 0660, NULL,
	thp_phone_sleep_mode_status_store);
static DEVICE_ATTR(thp_magnification_info, 0660,
	thp_magnification_info_show, NULL);
static DEVICE_ATTR(daemon_version, 0440, thp_daemon_version_show, NULL);
#ifdef CONFIG_HISYSEVENT
static DEVICE_ATTR(thp_test_hisysevent, 0200, NULL, thp_hisysevent_test_store);
#endif

static struct attribute *thp_ts_attributes[] = {
	&dev_attr_thp_status.attr,
	&dev_attr_touch_chip_info.attr,
	&dev_attr_hostprocessing.attr,
	&dev_attr_loglevel.attr,
#if defined(THP_CHARGER_FB)
	&dev_attr_charger_state.attr,
#endif
	&dev_attr_roi_data.attr,
	&dev_attr_roi_data_internal.attr,
	&dev_attr_roi_enable.attr,
	&dev_attr_touch_sensitivity.attr,
	&dev_attr_touch_glove.attr,
	&dev_attr_touch_window.attr,
	&dev_attr_touch_switch.attr,
	&dev_attr_udfp_enable.attr,
	&dev_attr_supported_func_indicater.attr,
	&dev_attr_tui_wake_up_enable.attr,
	&dev_attr_easy_wakeup_gesture.attr,
	&dev_attr_wakeup_gesture_enable.attr,
	&dev_attr_easy_wakeup_control.attr,
	&dev_attr_easy_wakeup_position.attr,
	&dev_attr_touch_oem_info.attr,
	&dev_attr_stylus_wakeup_ctrl.attr,
	&dev_attr_fingerprint_switch_ctrl.attr,
	&dev_attr_ring_switch_ctrl.attr,
	&dev_attr_power_switch_ctrl.attr,
	&dev_attr_tp_dfx_info.attr,
	&dev_attr_aod_touch_switch_ctrl.attr,
	&dev_attr_ring_setting_config_ctrl.attr,
	&dev_attr_factory_extra_cmd.attr,
	&dev_attr_screen_status_switch.attr,
	&dev_attr_thp_app_info.attr,
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	&dev_attr_aod_event_report_ctrl.attr,
	&dev_attr_tp_ud_lowpower_ctrl.attr,
	&dev_attr_aod_notify_tp_power_ctrl.attr,
#endif
#if defined(CONFIG_LCD_KIT_DRIVER)
	&dev_attr_jkm_notify_tp_power_ctrl.attr,
#endif
	&dev_attr_phone_sleep_mode_status.attr,
	&dev_attr_thp_magnification_info.attr,
	&dev_attr_daemon_version.attr,
#ifdef CONFIG_HISYSEVENT
	&dev_attr_thp_test_hisysevent.attr,
#endif
	NULL,
};

static const struct attribute_group thp_ts_attr_group = {
	.attrs = thp_ts_attributes,
};

int creat_touchscreen(void)
{
	static int panel_num = 0;
	int rc;
	static struct platform_device *pdev = NULL;

	panel_num++;
	if ((panel_num < MULTI_PANEL_NUM) || (pdev != NULL)) {
		thp_log_err("%s: multi sysfs not creat ready\n", __func__);
		return 0;
	}

	pdev = platform_device_alloc("tp", -1);
	if (!pdev) {
		thp_log_err("%s: regist platform_device failed\n", __func__);
		return -ENODEV;
	}

	rc = platform_device_add(pdev);
	if (rc) {
		thp_log_err("%s: add platform_device failed\n", __func__);
		platform_device_unregister(pdev);
		return -ENODEV;
	}

	rc = sysfs_create_link(NULL, &pdev->dev.kobj,
		"touchscreen");
	if (rc) {
		thp_log_err("%s: fail create link error = %d\n", __func__, rc);
		goto err_create_link;
	}
	thp_log_info("%s:create link ok\n", __func__);
	return 0;
err_create_link:
	platform_device_put(pdev);
	platform_device_unregister(pdev);
	return rc;
}
EXPORT_SYMBOL(creat_touchscreen);

int thp_init_sysfs(struct thp_core_data *cd)
{
	int rc;
	int error = 0;
	char node[MULTI_PANEL_NODE_BUF_LEN] = {0};

	if (!cd) {
		thp_log_err("%s: core data null\n", __func__);
		return -EINVAL;
	}

	if (cd->panel_id == 0)
		cd->thp_platform_dev = platform_device_alloc("huawei_thp0", -1);
	else if (cd->panel_id == 1)
		cd->thp_platform_dev = platform_device_alloc("huawei_thp1", -1);
	if (!cd->thp_platform_dev) {
		thp_log_err("%s: regist platform_device failed\n", __func__);
		return -ENODEV;
	}

	rc = platform_device_add(cd->thp_platform_dev);
	if (rc) {
		thp_log_err("%s: add platform_device failed\n", __func__);
		platform_device_unregister(cd->thp_platform_dev);
		return -ENODEV;
	}

	rc = sysfs_create_group(&cd->thp_platform_dev->dev.kobj,
			&thp_ts_attr_group);
	if (rc) {
		thp_log_err("%s:can't create ts's sysfs\n", __func__);
		goto err_create_group;
	}

	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL) {
		rc = snprintf(node, MULTI_PANEL_NODE_BUF_LEN, "%s%d",
			SYSFS_TOUCH_PATH, cd->multi_panel_index);
		if (rc < 0) {
			thp_log_err("%s: snprintf err\n", __func__);
			goto err_create_group;
		}
		rc = sysfs_create_link(NULL, &cd->thp_platform_dev->dev.kobj,
			node);
		error = creat_touchscreen();
	} else {
		rc = sysfs_create_link(NULL, &cd->thp_platform_dev->dev.kobj,
			SYSFS_TOUCH_PATH);
	}
	if (rc + error) {
		thp_log_err("%s: fail create link error = %d %d\n", __func__, rc, error);
		rc += error;
		goto err_create_link;
	}

	return 0;

err_create_link:
	sysfs_remove_group(&cd->thp_platform_dev->dev.kobj, &thp_ts_attr_group);
err_create_group:
	platform_device_put(cd->thp_platform_dev);
	platform_device_unregister(cd->thp_platform_dev);
	return rc;
}


void thp_sysfs_release(struct thp_core_data *cd)
{
	if (!cd) {
		thp_log_err("%s: core data null\n", __func__);
		return;
	}
	platform_device_put(cd->thp_platform_dev);
	platform_device_unregister(cd->thp_platform_dev);
	sysfs_remove_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
}
