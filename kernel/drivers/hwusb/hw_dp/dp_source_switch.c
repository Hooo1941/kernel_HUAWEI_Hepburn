/*
 * dp_source_switch.c
 *
 * dp source switch for DP module
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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

#include "dp_source_switch.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/of.h>
#include <huawei_platform/log/hw_log.h>
#include "dp_dsm.h"
#include "dp_factory.h"

#define HWLOG_TAG dp_source_switch
HWLOG_REGIST();

#ifndef unused
#define unused(x) (void)(x)
#endif

#ifndef min
#define min(x, y)  ((x) < (y) ? (x) : (y))
#endif

#define EDID_BLOCK_LENGTH 128
#define EDID_LINE_LENGTH  16

enum dp_resolution_type {
	RESOLUTION_640_480_60FPS = 1,
	RESOLUTION_800_600_60FPS,
	RESOLUTION_800_600_75FPS,
	RESOLUTION_1024_768_60FPS,
	RESOLUTION_1280_768_60FPS_CVT,
	RESOLUTION_1280_768_60FPS,
	RESOLUTION_1280_800_60FPS_CVT,
	RESOLUTION_1280_800_60FPS,
	RESOLUTION_1280_960_60FPS,
	RESOLUTION_1280_1024_60FPS,
	RESOLUTION_1360_768_60FPS,
	RESOLUTION_1400_1050_60FPS,
	RESOLUTION_1600_1200_60FPS_CVT,
	RESOLUTION_1600_1200_60FPS,
	RESOLUTION_1680_1050_60FPS_CVT,
	RESOLUTION_1680_1050_60FPS,
	RESOLUTION_1792_1344_60FPS,
	RESOLUTION_1920_1080_60FPS,
	RESOLUTION_1920_1200_60FPS,
	RESOLUTION_2048_1536_60FPS,
	RESOLUTION_2560_1600_60FPS_CVT,
	RESOLUTION_2560_1600_60FPS,
	RESOLUTION_1440_480_59FPS,
	RESOLUTION_1440_576_50FPS,
	RESOLUTION_3840_2160_60FPS,
	RESOLUTION_1920_1080_60FPS_CEA,
	RESOLUTION_3840_2160_30FPS,
	RESOLUTION_MAX,
};

enum dp_lcd_power_type {
	LCD_POWER_OFF = 0,
	LCD_POWER_ON,
};

enum dp_video_format_type {
	VCEA = 0,
	CVT,
	DMT,
};

/* choose VR or PC Mode */
enum dp_external_disp_type {
	EXT_PC_MODE = 0,
	EXT_VR_MODE,
	EXT_UNDEF,
};

struct dp_external_info {
	uint16_t width;
	uint16_t high;
	uint16_t fps;
};

struct dp_resolution_info {
	enum dp_resolution_type r_type;
	enum dp_video_format_type v_type;
	uint8_t  vesa_id;
	uint16_t with;
	uint16_t high;
	uint16_t fps;
};

struct dp_source_priv {
	struct class *class;
	bool parse_dts_flag;
	bool support_switch_source_mode;

	enum dp_source_mode source_mode;
	enum dp_lcd_power_type lcd_power_type;
	struct dp_external_info external_info;
	enum dp_external_disp_type ex_disp_type;
	struct dp_resolution_info resolution_switch;
	uint8_t edid[EDID_BLOCK_LENGTH];

	switch_source_handler switch_hdl;
};

struct dp_link_state_to_event {
	enum dp_link_state state;
	char *event;
};

struct dp_device_type {
	const char *name;
	struct device *dev;
	int index;
};

static struct dp_device_type dp_source = {
	.name = "source",
	.index = 0,
};

static struct dp_device_type dp_resolution = {
	.name = "resolution",
	.index = 0,
};

static struct dp_device_type dp_vr = {
	.name = "vr",
	.index = 0,
};

static struct dp_device_type dp_ex_info = {
	.name = "external_display",
	.index = 0,
};

static struct dp_device_type dp_lcd_power = {
	.name = "power",
	.index = 0,
};

static struct dp_device_type dp_edid = {
	.name = "edid",
	.index = 0,
};

/* NOTE: need to be treated separately for normal or factory version */
static struct dp_link_state_to_event g_dp_link_event[] = {
#ifndef DP_FACTORY_MODE_ENABLE
	/* for normal version */
	{ DP_LINK_STATE_CABLE_IN, "DP_LINK_EVENT=CABLE_IN" },
	{ DP_LINK_STATE_CABLE_OUT, "DP_LINK_EVENT=CABLE_OUT" },
	{ DP_LINK_STATE_MULTI_HPD, "DP_LINK_EVENT=MULTI_HPD" },
	{ DP_LINK_STATE_AUX_FAILED, "DP_LINK_EVENT=AUX_FAILED" },
	{ DP_LINK_STATE_SAFE_MODE, "DP_LINK_EVENT=SAFE_MODE" },
	{ DP_LINK_STATE_EDID_FAILED, "DP_LINK_EVENT=EDID_FAILED" },
	{ DP_LINK_STATE_LINK_FAILED, "DP_LINK_EVENT=LINK_FAILED" },
	{ DP_LINK_STATE_LINK_RETRAINING, "DP_LINK_EVENT=LINK_RETRAINING" },
	{ DP_LINK_STATE_HDCP_FAILED, "DP_LINK_EVENT=HDCP_FAILED" },
#else
	/* for factory version: MMIE test */
	{ DP_LINK_STATE_CABLE_IN, "MANUFACTURE_DP_LINK_EVENT=CABLE_IN" },
	{ DP_LINK_STATE_CABLE_OUT, "MANUFACTURE_DP_LINK_EVENT=CABLE_OUT" },
	{ DP_LINK_STATE_AUX_FAILED, "MANUFACTURE_DP_LINK_EVENT=AUX_FAILED" },
	{ DP_LINK_STATE_SAFE_MODE, "MANUFACTURE_DP_LINK_EVENT=SAFE_MODE" },
	{ DP_LINK_STATE_EDID_FAILED, "MANUFACTURE_DP_LINK_EVENT=EDID_FAILED" },
	{ DP_LINK_STATE_LINK_FAILED, "MANUFACTURE_DP_LINK_EVENT=LINK_FAILED" },
	{ DP_LINK_STATE_HPD_NOT_EXISTED, "MANUFACTURE_DP_LINK_EVENT=HPD_NOT_EXISTED" },
	{ DP_LINK_STATE_LINK_REDUCE_RATE, "MANUFACTURE_DP_LINK_EVENT=LINK_REDUCE_RATE" },
	{ DP_LINK_STATE_INVALID_COMBINATIONS, "MANUFACTURE_DP_LINK_EVENT=INVALID_COMBINATIONS" },
#endif /* DP_FACTORY_MODE_ENABLE */
};

static struct dp_resolution_info resolution_map[] = {
	{ RESOLUTION_640_480_60FPS,       CVT,   1,  640,  480, 60, },
	{ RESOLUTION_800_600_60FPS,       CVT,   2,  800,  600, 60, },
	{ RESOLUTION_800_600_75FPS,       DMT,  11,  800,  600, 75, },
	{ RESOLUTION_1024_768_60FPS,      CVT,   3, 1024,  768, 60, },
	{ RESOLUTION_1280_768_60FPS_CVT,  DMT,  22, 1280,  768, 60, },
	{ RESOLUTION_1280_768_60FPS,      CVT,  13, 1280,  768, 60, },
	{ RESOLUTION_1280_800_60FPS_CVT,  DMT,  27, 1280,  800, 60, },
	{ RESOLUTION_1280_800_60FPS,      CVT,  28, 1280,  800, 60, },
	{ RESOLUTION_1280_960_60FPS,      CVT,   4, 1280,  960, 60, },
	{ RESOLUTION_1280_1024_60FPS,     CVT,  12, 1280, 1024, 60, },
	{ RESOLUTION_1360_768_60FPS,      CVT,  17, 1360,  768, 60, },
	{ RESOLUTION_1400_1050_60FPS,     DMT,  41, 1440, 1050, 60, },
	{ RESOLUTION_1600_1200_60FPS_CVT, CVT,  40, 1600, 1200, 60, },
	{ RESOLUTION_1600_1200_60FPS,     CVT,   6, 1600, 1200, 60, },
	{ RESOLUTION_1680_1050_60FPS_CVT, DMT,  57, 1680, 1050, 60, },
	{ RESOLUTION_1680_1050_60FPS,     DMT,  58, 1680, 1050, 60, },
	{ RESOLUTION_1792_1344_60FPS,     DMT,  62, 1792, 1344, 60, },
	{ RESOLUTION_1920_1080_60FPS,     CVT,  20, 1920, 1080, 60, },
	{ RESOLUTION_1920_1200_60FPS,     CVT,  34, 1920, 1200, 60, },
	{ RESOLUTION_2048_1536_60FPS,     CVT,  41, 2048, 1536, 60, },
	{ RESOLUTION_2560_1600_60FPS_CVT, DMT,  76, 2560, 1600, 60, },
	{ RESOLUTION_2560_1600_60FPS,     DMT,  77, 2560, 1600, 60, },
	{ RESOLUTION_1440_480_59FPS,      VCEA, 15, 1440,  480, 59, },
	{ RESOLUTION_1440_576_50FPS,      VCEA, 30, 1440,  576, 50, },
	{ RESOLUTION_3840_2160_60FPS,     VCEA, 97, 3840, 2160, 60, },
	{ RESOLUTION_1920_1080_60FPS_CEA, VCEA, 16, 1920, 1080, 60, },
	{ RESOLUTION_3840_2160_30FPS,     VCEA, 95, 3840, 2160, 30, },
};

static struct dp_source_priv g_dp_priv;

static void dp_source_mode_parse_dts(void);

__attribute__ ((weak)) int dpu_dptx_switch_source(uint32_t user_mode,
	uint32_t user_format)
{
	unused(user_mode);
	unused(user_format);
	hwlog_info("%s: use weak\n", __func__);
	return 0;
}

__attribute__ ((weak)) int dpu_dptx_main_panel_blank(bool bblank)
{
	unused(bblank);
	hwlog_info("%s: use weak\n", __func__);
	return 0;
}

int set_external_display_type(uint32_t type)
{
	if (type > EXT_UNDEF) {
		hwlog_err("%s: not support type %d, please check it\n", __func__, type);
		return EXT_UNDEF;
	}

	g_dp_priv.resolution_switch.r_type = (enum dp_resolution_type)type;
	return 0;
}
EXPORT_SYMBOL_GPL(set_external_display_type);

int update_external_display_timming_info(uint32_t width,
	uint32_t high, uint32_t fps)
{
	g_dp_priv.external_info.width = width;
	g_dp_priv.external_info.high = high;
	g_dp_priv.external_info.fps = fps;
	return 0;
}
EXPORT_SYMBOL_GPL(update_external_display_timming_info);

int get_current_dp_source_mode(void)
{
	if (!g_dp_priv.parse_dts_flag)
		dp_source_mode_parse_dts();

	return g_dp_priv.source_mode;
}
EXPORT_SYMBOL_GPL(get_current_dp_source_mode);

int save_dp_edid(const uint8_t *edid_buf, uint32_t buf_len)
{
	if (!edid_buf || (buf_len != EDID_BLOCK_LENGTH)) {
		hwlog_err("%s: edid_buf error\n", __func__);
		memset(g_dp_priv.edid, 0, EDID_BLOCK_LENGTH);
		return -EINVAL;
	}
	memcpy(g_dp_priv.edid, edid_buf, EDID_BLOCK_LENGTH);

	return 0;
}
EXPORT_SYMBOL_GPL(save_dp_edid);

void register_switch_handler(switch_source_handler handler)
{
	g_dp_priv.switch_hdl = handler;
	hwlog_info("%s\n", __func__);
}
EXPORT_SYMBOL_GPL(register_switch_handler);

static ssize_t dp_source_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_err("%s: buf is null\n", __func__);
		return -EINVAL;
	}

	ret = scnprintf(buf, PAGE_SIZE, "%d\n", g_dp_priv.source_mode);
	return ret;
}

static void dp_source_mode_parse_dts(void)
{
	struct device_node *np = NULL;
	int value = 0;
	int ret;

	hwlog_info("%s: enter\n", __func__);
	if (g_dp_priv.parse_dts_flag) {
		hwlog_info("%s: source mode has already parsed\n", __func__);
		return;
	}
	g_dp_priv.parse_dts_flag = true;

	np = of_find_compatible_node(NULL, NULL, "huawei,dp_source_switch");
	if (!np) {
		hwlog_info("%s: not found dts node\n", __func__);
		/* If no node find will setting the defalut mode as SAME_SOURCE */
		g_dp_priv.source_mode = SAME_SOURCE;
		return;
	}
	g_dp_priv.support_switch_source_mode = true;

	if (!of_device_is_available(np)) {
		hwlog_err("%s: dts %s not available\n", __func__, np->name);
		g_dp_priv.source_mode = SAME_SOURCE;
		return;
	}

	ret = of_property_read_u32(np, "dp_default_source_mode", &value);
	if (ret) {
		hwlog_info("%s: get default source_mode failed %d\n",
			__func__, ret);
		g_dp_priv.source_mode = SAME_SOURCE;
		return;
	}

	if (!value)
		g_dp_priv.source_mode = DIFF_SOURCE;
	else
		g_dp_priv.source_mode = SAME_SOURCE;

	if (dp_factory_mode_is_enable()) {
		g_dp_priv.source_mode = SAME_SOURCE;
		hwlog_info("%s: only support same source in factory version\n", __func__);
	}

	hwlog_info("%s: get source mode %d success\n", __func__,
		g_dp_priv.source_mode);
}

static ssize_t dp_source_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int last_mode;
	int ret;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_err("%s: buf is null\n", __func__);
		return -EINVAL;
	}
	if (!g_dp_priv.support_switch_source_mode)
		return count;

	last_mode = g_dp_priv.source_mode;
	ret = kstrtoint(buf, 0, (int *)(&g_dp_priv.source_mode));
	if (ret) {
		hwlog_err("%s: store error\n", __func__);
		return -EINVAL;
	}

	if ((g_dp_priv.source_mode == SAME_SOURCE) ||
		(g_dp_priv.source_mode == DIFF_SOURCE)) {
		if (last_mode == g_dp_priv.source_mode) {
			hwlog_info("%s: sync framework source mode state %d\n",
				__func__, g_dp_priv.source_mode);
			return count;
		}

		hwlog_info("%s: store %d success\n", __func__, g_dp_priv.source_mode);
		dp_imonitor_set_param(DP_PARAM_SOURCE_SWITCH, &g_dp_priv.source_mode);

		if (g_dp_priv.switch_hdl != NULL)
			g_dp_priv.switch_hdl();
		else
			dpu_dptx_switch_source(0, 0);
	} else {
		hwlog_err("%s: invalid source_mode %d\n", __func__,
			g_dp_priv.source_mode);
	}

	return count;
}

static bool dp_find_resolution_type(int type)
{
	int event_size = ARRAY_SIZE(g_dp_link_event);
	bool find = false;
	int i;

	for (i = 0; i < event_size; i++) {
		if (resolution_map[i].r_type != type)
			continue;

		find = true;
		g_dp_priv.resolution_switch.v_type = resolution_map[i].v_type;
		g_dp_priv.resolution_switch.vesa_id = resolution_map[i].vesa_id;
		g_dp_priv.resolution_switch.with = resolution_map[i].with;
		g_dp_priv.resolution_switch.high = resolution_map[i].high;
		g_dp_priv.resolution_switch.fps = resolution_map[i].fps;
	}
	return find;
}

static ssize_t dp_source_resolution_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_err("%s: buf is null\n", __func__);
		return -EINVAL;
	}

	ret = scnprintf(buf, PAGE_SIZE, "%d\n", g_dp_priv.resolution_switch.r_type);
	return ret;
}

static ssize_t dp_source_resolution_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int type = 0;
	int ret;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_err("%s: buf is null\n", __func__);
		return -EINVAL;
	}

	if (!g_dp_priv.support_switch_source_mode)
		return count;
	hwlog_info("%s: enter buf %s\n", __func__, buf);

	ret = kstrtoint(buf, 0, &type);
	if (ret) {
		hwlog_err("%s: store error\n", __func__);
		return -EINVAL;
	}

	if (type >= RESOLUTION_MAX) {
		hwlog_err("%s: not support resolution %d, please check it\n",
			__func__, type);
		goto err_out;
	}

	g_dp_priv.resolution_switch.r_type = type;

	if (!dp_find_resolution_type(type)) {
		hwlog_err("%s: not find this resolution %d\n", __func__, type);
		goto err_out;
	}

	if (g_dp_priv.switch_hdl != NULL)
		ret = g_dp_priv.switch_hdl();
	else
		ret = dpu_dptx_switch_source(g_dp_priv.resolution_switch.vesa_id,
			(int)g_dp_priv.resolution_switch.v_type);

	if (ret)
		hwlog_info("%s: dptx switch source failed\n", __func__);

err_out:
	hwlog_info("%s: with=%d, high=%d, fps=%d, v_type=%d, vesa_id=%d\n", __func__,
		g_dp_priv.resolution_switch.with, g_dp_priv.resolution_switch.high,
		g_dp_priv.resolution_switch.fps, g_dp_priv.resolution_switch.v_type,
		g_dp_priv.resolution_switch.vesa_id);

	return count;
}

static ssize_t dp_vr_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_info("%s: buf is null\n", __func__);
		return -EINVAL;
	}

	ret = scnprintf(buf, PAGE_SIZE, "%d\n", g_dp_priv.ex_disp_type);
	return ret;
}

static ssize_t dp_lcd_power_show(struct device *dev,
	 struct device_attribute *attr, char *buf)
{
	int ret;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_info("%s: buf is null\n", __func__);
		return -EINVAL;
	}

	ret = scnprintf(buf, PAGE_SIZE, "%d\n", g_dp_priv.lcd_power_type);
	return ret;
}

static ssize_t dp_lcd_power_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_info("%s: buf is null\n", __func__);
		ret = -EINVAL;
		return ret;
	}

	if (!g_dp_priv.support_switch_source_mode)
		return count;

	ret = kstrtoint(buf, 0, (int *)(&g_dp_priv.lcd_power_type));
	if (ret) {
		hwlog_info("%s: store error %d\n", __func__, ret);
		return -EINVAL;
	}

	if ((g_dp_priv.lcd_power_type == LCD_POWER_OFF) ||
		(g_dp_priv.lcd_power_type == LCD_POWER_ON)) {
		if (g_dp_priv.source_mode == DIFF_SOURCE) {
			hwlog_info("%s: store %d success\n", __func__,
				g_dp_priv.lcd_power_type);
			dpu_dptx_main_panel_blank(g_dp_priv.lcd_power_type);
		} else {
			hwlog_info("%s: same mode, do nothing\n", __func__);
		}
	} else {
		hwlog_info("%s: invalid lcd_power %d\n", __func__,
			g_dp_priv.lcd_power_type);
	}

	return count;
}

static ssize_t dp_external_display_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_info("%s: buf is null\n", __func__);
		return -EINVAL;
	}

	if ((g_dp_priv.external_info.width == 0) ||
		(g_dp_priv.external_info.high == 0))
		ret = scnprintf(buf, PAGE_SIZE, "%s\n", "No External Display");
	else
		ret = scnprintf(buf, PAGE_SIZE,
			"External Display Info: Width=%4d.High=%4d.FPS=%2d\n",
			g_dp_priv.external_info.width,
			g_dp_priv.external_info.high,
			g_dp_priv.external_info.fps);

	return ret;
}

static ssize_t dp_edid_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len = 0;
	int i;

	unused(dev);
	unused(attr);
	if (!buf) {
		hwlog_info("%s: buf is null\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < EDID_BLOCK_LENGTH; i++) {
		len += scnprintf(buf + len, PAGE_SIZE - len, "%02x ", g_dp_priv.edid[i]);
		if ((i + 1) % EDID_LINE_LENGTH == 0)
			len += scnprintf(buf + len, PAGE_SIZE - len, "\n");
	}
	hwlog_info("edid len = %d\n", len);

	return len;
}

void dp_send_event(enum dp_event_type event)
{
	char event_buf[DP_LINK_EVENT_BUF_MAX] = { 0 };
	char *envp[] = { event_buf, NULL };
	int ret;

	switch (event) {
	case DP_LINK_STATE_GOOD:
		snprintf(event_buf, sizeof(event_buf), "DP_LINK_STATE=GOOD");
		break;
	case DP_LINK_STATE_BAD:
		snprintf(event_buf, sizeof(event_buf), "DP_LINK_STATE=BAD");
		break;
	default:
		return;
	}

	ret = kobject_uevent_env(&dp_vr.dev->kobj, KOBJ_CHANGE, envp);
	if (ret < 0)
		hwlog_err("%s: send uevent failed %d\n", __func__, ret);
	else
		hwlog_info("%s: send uevent %s success\n", __func__, envp[0]);
}
EXPORT_SYMBOL_GPL(dp_send_event);

void dp_link_state_event(enum dp_link_state state, bool dptx_vr_flag)
{
	char event_buf[DP_LINK_EVENT_BUF_MAX] = { 0 };
	char *envp[] = { event_buf, NULL };
	int count = ARRAY_SIZE(g_dp_link_event);
	bool find = false;
	int size;
	int ret;
	int i;

	if (state >= DP_LINK_STATE_MAX) {
		hwlog_err("%s: invalid link state %d\n", __func__, state);
		return;
	}

	if (dptx_vr_flag) {
		hwlog_info("%s: The display is VR, not report event\n", __func__);
		return;
	}

	for (i = 0; i < count; i++) {
		if ((g_dp_link_event[i].state == state) && g_dp_link_event[i].event &&
			strlen(g_dp_link_event[i].event)) {
			find = true;
			size = min(DP_LINK_EVENT_BUF_MAX - 1,
				(int)strlen(g_dp_link_event[i].event));
			strncpy(event_buf, g_dp_link_event[i].event, size);
			dp_factory_set_link_event_no(state, false, event_buf, 0);
			break;
		}
	}

	if (!find) {
		hwlog_err("%s: not find link event %d\n", __func__, state);
		return;
	}

	if (dp_factory_mode_is_enable() && !dp_factory_need_report_event()) {
		hwlog_info("%s: skip %s in factory\n", __func__, event_buf);
		return;
	}

	ret = kobject_uevent_env(&dp_source.dev->kobj, KOBJ_CHANGE, envp);
	if (ret < 0)
		hwlog_err("%s: report %s failed %d\n", __func__, event_buf, ret);
	else
		hwlog_info("%s: report %s success\n", __func__, event_buf);
}
EXPORT_SYMBOL_GPL(dp_link_state_event);

static struct device_attribute dev_source_mode =
	__ATTR(source_mode, 0644, dp_source_mode_show, dp_source_mode_store);
static struct device_attribute dev_source_resolution =
	__ATTR(source_resolution, 0644,
	dp_source_resolution_show, dp_source_resolution_store);
static struct device_attribute dev_vr_mode =
	__ATTR(vr_mode, 0440, dp_vr_mode_show, NULL);
static struct device_attribute dev_external_display =
	__ATTR(display_info, 0440, dp_external_display_show, NULL);
static struct device_attribute dev_lcd_power =
	__ATTR(lcd_power, 0644, dp_lcd_power_show, dp_lcd_power_store);
static struct device_attribute dev_edid =
	__ATTR(edid_info, 0440, dp_edid_show, NULL);

static int dp_source_mode_create_file(void)
{
	int ret;

	ret = device_create_file(dp_source.dev, &dev_source_mode);
	if (ret) {
		hwlog_err("%s: dp_source create failed\n", __func__);
		goto err_out;
	}

	ret = device_create_file(dp_resolution.dev, &dev_source_resolution);
	if (ret) {
		hwlog_err("%s: dp_resolution create failed\n", __func__);
		goto err_out_source;
	}

	ret = device_create_file(dp_vr.dev, &dev_vr_mode);
	if (ret) {
		hwlog_err("%s: dp_vr create failed\n", __func__);
		goto err_out_resolution;
	}

	ret = device_create_file(dp_ex_info.dev, &dev_external_display);
	if (ret) {
		hwlog_err("%s: dp_ex_info create failed\n", __func__);
		goto err_out_vr;
	}

	ret = device_create_file(dp_lcd_power.dev, &dev_lcd_power);
	if (ret) {
		hwlog_err("%s: dp_lcd_power create failed\n", __func__);
		goto err_out_externel_info;
	}

	ret = device_create_file(dp_edid.dev, &dev_edid);
	if (ret) {
		hwlog_err("%s: dp_edid create failed\n", __func__);
		goto err_out_lcd;
	}

	hwlog_info("%s: create success\n", __func__);
	return 0;

err_out_lcd:
	device_remove_file(dp_lcd_power.dev, &dev_lcd_power);
err_out_externel_info:
	device_remove_file(dp_ex_info.dev, &dev_external_display);
err_out_vr:
	device_remove_file(dp_vr.dev, &dev_vr_mode);
err_out_resolution:
	device_remove_file(dp_resolution.dev, &dev_source_resolution);
err_out_source:
	device_remove_file(dp_source.dev, &dev_source_mode);
err_out:
	hwlog_err("%s: create failed %d\n", __func__, ret);
	return ret;
}

#define dp_device_create(priv, type) \
	type.dev = device_create(priv.class, NULL, 0, NULL, type.name)

#define dp_device_destroy(priv, type) \
do { \
	if (!IS_ERR_OR_NULL(type.dev)) { \
		device_destroy(priv.class, type.dev->devt); \
		type.dev = NULL; \
	} \
} while (0)

#define dp_remove_file(priv, type, attr) \
do { \
	if (!IS_ERR_OR_NULL(type.dev)) { \
		device_remove_file(type.dev, &attr); \
		device_destroy(priv.class, type.dev->devt); \
		type.dev = NULL; \
	} \
} while (0)

static int __init dp_source_mode_init(void)
{
	struct class *dp_class = NULL;
	int ret;

	hwlog_info("%s: enter\n", __func__);
	memset(&g_dp_priv, 0, sizeof(g_dp_priv));

	g_dp_priv.support_switch_source_mode = false;
	g_dp_priv.parse_dts_flag = false;
	g_dp_priv.source_mode = SAME_SOURCE;
	g_dp_priv.ex_disp_type = EXT_PC_MODE;
	g_dp_priv.lcd_power_type = LCD_POWER_OFF;
	dp_source_mode_parse_dts();

	dp_class = class_create(THIS_MODULE, "dp");
	if (IS_ERR(dp_class)) {
		hwlog_err("%s: create dp source class failed\n", __func__);
		return IS_ERR(dp_class);
	}
	g_dp_priv.class = dp_class;

	dp_device_create(g_dp_priv, dp_source);
	dp_device_create(g_dp_priv, dp_resolution);
	dp_device_create(g_dp_priv, dp_vr);
	dp_device_create(g_dp_priv, dp_ex_info);
	dp_device_create(g_dp_priv, dp_lcd_power);
	dp_device_create(g_dp_priv, dp_edid);
	if (IS_ERR_OR_NULL(dp_source.dev) || IS_ERR_OR_NULL(dp_resolution.dev) ||
		IS_ERR_OR_NULL(dp_vr.dev) || IS_ERR_OR_NULL(dp_ex_info.dev) ||
		IS_ERR_OR_NULL(dp_lcd_power.dev) || IS_ERR_OR_NULL(dp_edid.dev)) {
		hwlog_err("%s: some device create failed\n", __func__);
		ret = -EINVAL;
		goto err_out;
	}

	ret = dp_source_mode_create_file();
	if (ret < 0)
		goto err_out;

	dp_dsm_init();
	dp_factory_init();
	hwlog_info("%s: init success\n", __func__);
	return 0;

err_out:
	dp_device_destroy(g_dp_priv, dp_source);
	dp_device_destroy(g_dp_priv, dp_resolution);
	dp_device_destroy(g_dp_priv, dp_vr);
	dp_device_destroy(g_dp_priv, dp_ex_info);
	dp_device_destroy(g_dp_priv, dp_lcd_power);
	dp_device_destroy(g_dp_priv, dp_edid);

	class_destroy(g_dp_priv.class);
	g_dp_priv.class = NULL;

	hwlog_err("%s: init failed %d\n", __func__, ret);
	return ret;
}

static void __exit dp_source_mode_exit(void)
{
	hwlog_info("%s enter\n", __func__);
	dp_factory_exit();
	dp_dsm_exit();
	dp_remove_file(g_dp_priv, dp_source, dev_source_mode);
	dp_remove_file(g_dp_priv, dp_resolution, dev_source_resolution);
	dp_remove_file(g_dp_priv, dp_vr, dev_vr_mode);
	dp_remove_file(g_dp_priv, dp_ex_info, dev_external_display);
	dp_remove_file(g_dp_priv, dp_lcd_power, dev_source_mode);
	dp_remove_file(g_dp_priv, dp_edid, dev_edid);

	class_destroy(g_dp_priv.class);
	g_dp_priv.class = NULL;
}

module_init(dp_source_mode_init);
module_exit(dp_source_mode_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Huawei dp source driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
