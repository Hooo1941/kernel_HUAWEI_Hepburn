/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als channel source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_channel.h"

#include <linux/err.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "als_para_table.h"
#include "als_para_table_ams.h"
#include "als_para_table_ams_tcs3701.h"
#include "als_para_table_ams_tcs3718.h"
#include "als_para_table_ams_tmd3702.h"
#include "als_para_table_ams_tmd3725.h"
#include "als_para_table_ams_tsl2585.h"
#include "als_para_table_ams_tsl2591.h"
#include "als_para_table_avago.h"
#include "als_para_table_liteon.h"
#include "als_para_table_rohm.h"
#include "als_para_table_sensortek.h"
#include "als_para_table_sensortek_stk3638.h"
#include "als_para_table_silergy.h"
#include "als_para_table_sip1225.h"
#include "als_para_table_sip1226.h"
#include "als_para_table_st.h"
#include "als_para_table_vishay.h"
#include "als_sysfs.h"
#include "als_tp_color.h"
#include "contexthub_boot.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#ifdef CONFIG_CONTEXTHUB_SHMEM
#include "shmem.h"
#endif
#ifdef ALS_PANEL_VERSION
#include <lcd_kit_disp.h>
#endif

const char *g_als_nv_info[] = {"ALSTP1", "ALSTP2", "ALSTP3", "ALS_CALIDATA1", "ALS_CALIDATA2", "ALS_CALIDATA3"};
const int g_als_nv_num[] = {
	ALS_TP_CALIDATA_NV1_NUM, ALS_TP_CALIDATA_NV2_NUM, ALS_TP_CALIDATA_NV3_NUM,
	ALS_TP_CALIDATA_NV4_NUM, ALS_TP_CALIDATA_NV5_NUM, ALS_TP_CALIDATA_NV6_NUM,
};

static lcd_t lcd_info[] = {
	{"190", LCD_BOE},
	{"192", LCD_BOE2},
	{"310", LCD_VISIONOX},
	{"290", LCD_HUAXING}
};
static ddic_t ddic_info[] = {
	{"h01", DDIC_HISI},
	{"c10", DDIC_RUIDING},
	{"c08", DDIC_RUIDING692H5},
	{"c07", DDIC_RUIDING692H0},
	{"c06", DDIC_RUIDING},
	{"207", DDIC_NOVATEK},
	{"f01", DDIC_YUNYINGGU},
	{"d06", DDIC_JICHUANG}
};

unsigned int g_als_chip_normal[] = {
	ALS_CHIP_VISHAY_VCNL36832, ALS_CHIP_STK3338, ALS_CHIP_LTR2568,
	ALS_CHIP_VEML32185, ALS_CHIP_STK32670, ALS_CHIP_STK32671,
	ALS_CHIP_LTR311, ALS_CHIP_SYH399, ALS_CHIP_STK37670,
	ALS_CHIP_TSL2585, ALS_CHIP_SIP1225, ALS_CHIP_SIP1226
};

static DEFINE_MUTEX(mutex_set_para);

int8_t get_lcd(int32_t tag)
{
	uint8_t index;
	struct device_node *np = NULL;
	const char *lcd_model = NULL;
	struct als_platform_data *pf_data = NULL;

	pf_data = als_get_platform_data(tag);
	if (!pf_data) {
		hwlog_err("%s: als under tp no tag %d\n", __func__, tag);
		return LCD_OTHER;
	}
	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("als not find lcd_panel_type node\n");
		return LCD_OTHER;
	}
	hwlog_info("als_phone_type = %d, tag = %d\n", pf_data->als_phone_type, tag);
	if ((pf_data->als_phone_type == ALTB && tag == TAG_ALS) ||
		(pf_data->als_phone_type == LEM && tag == TAG_ALS1)) {
		if (of_property_read_string(np, "sec_lcd_panel_type", &lcd_model)) {
			hwlog_info("als not find sec_lcd_panel_type in dts\n");
			return LCD_OTHER;
		}
	} else {
		if (of_property_read_string(np, "lcd_panel_type", &lcd_model)) {
			hwlog_err("als not find lcd_panel_type in dts\n");
			return LCD_OTHER;
		}
	}
	hwlog_info("tag = %d, lcd_panel_type: %s suc in dts!!\n", tag, lcd_model);
	for (index = 0; index < ARRAY_SIZE(lcd_info); index++) {
		if (!strncmp(lcd_model, lcd_info[index].lcd_model, strlen(lcd_info[index].lcd_model))) {
			hwlog_info("%s: lcd = %d\n", __func__, lcd_info[index].lcd);
			return lcd_info[index].lcd;
		}
	}
	return LCD_OTHER;
}
int8_t get_ddic(int32_t tag)
{
	uint8_t index;
	struct device_node *np = NULL;
	const char *lcd_model = NULL;
	struct als_platform_data *pf_data = NULL;

	pf_data = als_get_platform_data(tag);
	if (!pf_data) {
		hwlog_err("%s: als under tp no tag %d\n", __func__, tag);
		return UNKNOWN_DDIC;
	}
	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("als not find lcd_panel_type node\n");
		return UNKNOWN_DDIC;
	}
	if ((pf_data->als_phone_type == ALTB && tag == TAG_ALS) ||
		(pf_data->als_phone_type == LEM && tag == TAG_ALS1)) {
		if (of_property_read_string(np, "sec_lcd_panel_type", &lcd_model)) {
			hwlog_info("als not find sec_lcd_panel_type in dts\n");
			return UNKNOWN_DDIC;
		}
	} else {
		if (of_property_read_string(np, "lcd_panel_type", &lcd_model)) {
			hwlog_err("als not find lcd_panel_type in dts\n");
			return UNKNOWN_DDIC;
		}
	}
	for (index = 0; index < ARRAY_SIZE(ddic_info); index++) {
		if (!strncmp((lcd_model + DDIC_IDX), ddic_info[index].lcd_model, strlen(ddic_info[index].lcd_model))) {
			hwlog_info("%s: ddic = %d\n", __func__, ddic_info[index].ddic);
			return ddic_info[index].ddic;
		}
	}
	return UNKNOWN_DDIC;
}

int send_laya_als_calibrate_data_to_mcu(int32_t tag)
{
	struct als_device_info *dev_info = NULL;
	uint16_t als_dark_noise_offset;

	dev_info = als_get_device_info(tag);
	if ((tag != TAG_ALS) || (!dev_info)) {
		hwlog_err("%s: laya no tag %d\n", __func__, tag);
		return -1;
	}

	if (read_calibrate_data_from_nv(ALS_CALIDATA_NV_NUM,
		ALS_CALIDATA_NV_SIZE_WITH_DARK_NOISE_OFFSET, "LSENSOR"))
		return -1;

	dev_info->als_first_start_flag = 1;

	if (memcpy_s(dev_info->als_sensor_calibrate_data,
		sizeof(dev_info->als_sensor_calibrate_data),
		user_info.nv_data, ALS_CALIDATA_NV_SIZE_WITH_DARK_NOISE_OFFSET) != EOK)
		return -1;
	if (memcpy_s(dev_info->als_offset, sizeof(dev_info->als_offset),
		user_info.nv_data, sizeof(dev_info->als_offset)) != EOK)
		return -1;
	if (memcpy_s(&als_dark_noise_offset, sizeof(als_dark_noise_offset),
		user_info.nv_data + ALS_CALIDATA_NV_SIZE,
		sizeof(als_dark_noise_offset)) != EOK)
		return -1;

	hwlog_info("send_laya_als_calibrate_data_to_mcu %d %d %d %d %d %d %d\n",
		dev_info->als_offset[0], dev_info->als_offset[1],
		dev_info->als_offset[2], dev_info->als_offset[3],
		dev_info->als_offset[4], dev_info->als_offset[5],
		als_dark_noise_offset);

	if (send_calibrate_data_to_mcu(tag, SUB_CMD_SET_OFFSET_REQ,
		dev_info->als_sensor_calibrate_data,
		ALS_CALIDATA_NV_SIZE_WITH_DARK_NOISE_OFFSET, false))
		return -1;

	return 0;
}

#ifdef ALS_PANEL_VERSION
static void get_panel_version(int32_t tag)
{
	struct als_platform_data *pf_data = NULL;
	char *panel_version = NULL;
	panel_version = kzalloc(sizeof(char) * 32, GFP_ATOMIC);
	if (panel_version == NULL) {
		hwlog_err("%s: malloc panel_version failed.\n", __func__);
		return;
	}

	pf_data = als_get_platform_data(tag);
	if (!pf_data) {
		hwlog_err("%s: als under tp no tag %d\n", __func__, tag);
		return;
	}
	hwlog_info("als_phone_type = %d, tag = %d\n", pf_data->als_phone_type, tag);
	if ((pf_data->als_phone_type == ALTB && tag == TAG_ALS) ||
		(pf_data->als_phone_type == LEM && tag == TAG_ALS1)) {
		lcd_kit_sensor_notify_panel_version(LCD_PANEL_1_OUTSIDE, panel_version);
		hwlog_info("panel_version out: %s\n", panel_version);
	} else {
		lcd_kit_sensor_notify_panel_version(LCD_PANEL_MAIN, panel_version);
		hwlog_info("panel_version main: %s\n", panel_version);
	}
}
#endif

int send_als_calibrate_data_to_mcu(int32_t tag)
{
	uint32_t addr = 0;
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;

	pf_data = als_get_platform_data(tag);
	dev_info = als_get_device_info(tag);
#ifdef ALS_PANEL_VERSION
	get_panel_version(tag);
#endif
	if (!pf_data || !dev_info)
		return -1;

	if (pf_data->als_phone_type == LAYA)
		return send_laya_als_calibrate_data_to_mcu(tag);

	if (als_get_calidata_nv_addr(tag, &addr))
		return -1;
	if ((addr + ALS_CALIDATA_NV_ONE_SIZE) > ALS_CALIDATA_NV_TOTAL_SIZE)
		return -1;

	if (read_calibrate_data_from_nv(ALS_CALIDATA_NV_NUM,
		ALS_CALIDATA_NV_TOTAL_SIZE, "LSENSOR"))
		return -1;

	dev_info->als_first_start_flag = 1;

	if (memcpy_s(dev_info->als_offset, sizeof(dev_info->als_offset),
		user_info.nv_data + addr, sizeof(dev_info->als_offset)) != EOK)
		return -1;
	if (pf_data->als_phone_type != PCE && pf_data->als_phone_type != XYAO) {
		dev_info->als_offset[4] = get_lcd(tag);
		dev_info->als_offset[5] = get_ddic(tag);
	}
	if (memcpy_s(dev_info->als_sensor_calibrate_data,
		sizeof(dev_info->als_sensor_calibrate_data),
		dev_info->als_offset, ALS_CALIDATA_NV_SIZE) != EOK)
		return -1;
	hwlog_info("nve_direct_access read lsensor_offset %d %d %d %d %d %d\n",
		dev_info->als_offset[0], dev_info->als_offset[1],
		dev_info->als_offset[2], dev_info->als_offset[3],
		dev_info->als_offset[4], dev_info->als_offset[5]);

	if (read_calibrate_data_from_nv(ALS_CALIDATA_L_R_NUM,
		ALS_CALIDATA_L_R_SIZE, "FAC11")) {
		hwlog_info("read location nv fail\n");
		return -1;
	}

	dev_info->als_sensor_calibrate_data[ALS_CALIDATA_NV_SIZE] =
		*(uint8_t *)(user_info.nv_data + 4); /* 4 save location */
	hwlog_info("als sensor calibrate location data: %c\n",
		dev_info->als_sensor_calibrate_data[ALS_CALIDATA_NV_SIZE]);

	/* add 1 data save location */
	if (send_calibrate_data_to_mcu(tag, SUB_CMD_SET_OFFSET_REQ,
		dev_info->als_sensor_calibrate_data, ALS_CALIDATA_NV_SIZE + 1, false))
		return -1;

	return 0;
}

void als_ud_minus_dss_noise_send(int32_t tag,
	struct als_platform_data *pf_data, struct als_device_info *dev_info)
{
	if (pf_data->als_ud_type == ALS_UD_MINUS_DSS_NOISE) {
		dev_info->als_rgb_pa_to_sh = 1;
		send_calibrate_data_to_mcu(tag, SUB_CMD_SET_ALS_PA,
			(const void *)&dev_info->als_rgb_pa_to_sh,
			sizeof(dev_info->als_rgb_pa_to_sh), true);
	}
}
int als_under_tp_nv_cct_read(size_t cal_data_left, size_t read_len, uint8_t *buf)
{
	if (cal_data_left > 0) {
		read_len = (cal_data_left > ALS_TP_CALIDATA_NV_SIZE) ?
			ALS_TP_CALIDATA_NV_SIZE : cal_data_left;
		if (read_calibrate_data_from_nv(ALS_TP_CALIDATA_NV4_NUM, read_len,
			"ALS_CALIDATA1"))
			return -1;
		if (memcpy_s(buf, read_len, user_info.nv_data, read_len) != EOK)
			return -1;
		buf += read_len;
		cal_data_left -= read_len;
	}

	if (cal_data_left > 0) {
		read_len = (cal_data_left > ALS_TP_CALIDATA_NV_SIZE) ?
			ALS_TP_CALIDATA_NV_SIZE : cal_data_left;
		if (read_calibrate_data_from_nv(ALS_TP_CALIDATA_NV5_NUM, read_len,
			"ALS_CALIDATA2"))
			return -1;
		if (memcpy_s(buf, read_len, user_info.nv_data, read_len) != EOK)
			return -1;
		buf += read_len;
		cal_data_left -= read_len;
	}

	if (cal_data_left > 0) {
		read_len = (cal_data_left > ALS_TP_CALIDATA_NV_SIZE) ?
			sizeof(struct als_under_tp_calidata) -
			ALS_TP_CALIDATA_NV_SIZE * 5 : cal_data_left;
		if (read_calibrate_data_from_nv(ALS_TP_CALIDATA_NV6_NUM, read_len,
			"ALS_CALIDATA3"))
			return -1;
		if (memcpy_s(buf, read_len, user_info.nv_data, read_len) != EOK)
			return -1;
		buf += read_len;
		cal_data_left -= read_len;
	}
	return 0;
}


/* read underscreen als nv data */
int als_under_tp_nv_read(int32_t tag)
{
	uint8_t *buf = NULL;
	size_t cal_data_left = sizeof(struct als_under_tp_calidata);
	size_t read_len;
	struct als_device_info *dev_info = NULL;

	hwlog_info("%s tag %d enter\n", __func__, tag);

	dev_info = als_get_device_info(tag);
	if (!dev_info)
		return -1;

	buf = (uint8_t *)(&(dev_info->als_under_tp_cal_data));

	if (tag == TAG_ALS) {
		uint8_t nv_count = 0;
		while (nv_count < 3) {
			pr_info("%s %d time enter ALS1\n", __func__, nv_count);
			read_len = ((cal_data_left > ALS_TP_CALIDATA_NV_SIZE) ?
				ALS_TP_CALIDATA_NV_SIZE : cal_data_left);
			if (read_calibrate_data_from_nv(g_als_nv_num[nv_count], read_len, g_als_nv_info[nv_count]))
				return -1;
			if (memcpy_s(buf, read_len, user_info.nv_data, read_len) != EOK)
				return -1;
			buf += read_len;
			cal_data_left -= read_len;
			nv_count++;
		}
	} else if (tag == TAG_ALS1) {
		uint8_t nv_count = 3;
		while (nv_count < 6) {
			pr_info("%s %d time enter ALS2\n", __func__, nv_count);
			read_len = ((cal_data_left > ALS_TP_CALIDATA_NV_SIZE) ?
				ALS_TP_CALIDATA_NV_SIZE : cal_data_left);
			if (read_calibrate_data_from_nv(g_als_nv_num[nv_count], read_len, g_als_nv_info[nv_count]))
				return -1;
			if (memcpy_s(buf, read_len, user_info.nv_data, read_len) != EOK)
				return -1;
			buf += read_len;
			cal_data_left -= read_len;
			nv_count++;
		}
	}
	/* nv_type 1 add gamma cct */
	if ((cal_data_left > 0) && (dev_info->als_ud_nv_type == 1))
		als_under_tp_nv_cct_read(cal_data_left,read_len,buf);

	hwlog_info("%s: x = %d, y = %d, width = %d, len = %d\n", __func__,
		dev_info->als_under_tp_cal_data.x, dev_info->als_under_tp_cal_data.y,
		dev_info->als_under_tp_cal_data.width,
		dev_info->als_under_tp_cal_data.length);

	return 0;
}

int send_als_under_tp_calibrate_data_to_mcu(int32_t tag)
{
	int ret;
	int i;
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;

	pf_data = als_get_platform_data(tag);
	dev_info = als_get_device_info(tag);
	if ((!pf_data) || (!dev_info)) {
		hwlog_err("%s: als under tp no tag %d\n", __func__, tag);
		return -1;
	}

	if ((pf_data->als_phone_type != ELLE) &&
		(pf_data->als_phone_type != VOGUE) &&
		(pf_data->als_ud_type != ALS_UD_MINUS_TP_RGB) &&
		(pf_data->als_ud_type != ALS_UD_MINUS_DSS_NOISE))
		return -2; /* -2: no need under tp calibrate */

	als_ud_minus_dss_noise_send(tag, pf_data, dev_info);
	ret = als_under_tp_nv_read(tag);
	hwlog_info("%s: ret = %d, tag = %d, sizeof(als_under_tp_cal_data) = %lu\n",
		__func__, ret, tag, sizeof(dev_info->als_under_tp_cal_data));
	if (ret)
		return -1;
	/* nv_type 1 add gamma cct */
	if (dev_info->als_ud_nv_type == 1) {
		for (i = 0; i < ALS_RES_SIZE; i++)
			hwlog_info("als_under_tp_cal_data.res[%d] = %d\n",
				i, dev_info->als_under_tp_cal_data.nv_type_gamma.res[i]);
		for (i = 0; i < ALS_SUB_BLOCK_SIZE; i++)
			hwlog_info("als_under_tp_cal_data.sub_block[%d] = %d\n",
				i, dev_info->als_under_tp_cal_data.nv_type_gamma.sub_block[i]);
		for (i = 0; i < ALS_DBV_SIZE; i++)
			hwlog_info("als_under_tp_cal_data.dbv[%d] = %d\n",
				i, dev_info->als_under_tp_cal_data.nv_type_gamma.dbv[i]);
		for (i = 0; i < ALS_GAMMA_SIZE; i++)
			hwlog_info("als_under_tp_cal_data.gamma[%d] = %d\n",
				i, dev_info->als_under_tp_cal_data.nv_type_gamma.gamma[i]);
	} else {
		/* print als_under_tp_cal_data debug info */
		for (i = 0; i < ALS_UNDER_TP_A_COUNT; i++)
			hwlog_info("send_als_under_tp_calibrate_data: als_under_tp_cal_data.a[%d] = %d\n",
				i, dev_info->als_under_tp_cal_data.nv_type_defult.a[i]);
		for (i = 0; i < ALS_UNDER_TP_B_COUNT; i++)
			hwlog_info("send_als_under_tp_calibrate_data: als_under_tp_cal_data.b[%d] = %d\n",
				i, dev_info->als_under_tp_cal_data.nv_type_defult.b[i]);
	}

	dev_info->als_under_tp_first_start_flag = 1;
#ifdef CONFIG_CONTEXTHUB_SHMEM
	if (shmem_send(tag, &(dev_info->als_under_tp_cal_data),
		als_ud_nv_size(dev_info->als_ud_nv_type)))
		hwlog_err("%s shmem_send error\n", __func__);
#endif

	return 0;
}

int als_get_calidata_nv_addr(int32_t tag, uint32_t *addr)
{
	if (addr && (tag == TAG_ALS || tag == TAG_ALS1 || tag == TAG_ALS2)) {
		if (tag == TAG_ALS)
			*addr = 0;
		else if (tag == TAG_ALS1)
			*addr = ALS_CALIDATA_NV_ALS1_ADDR;
		else if (tag == TAG_ALS2)
			*addr = ALS_CALIDATA_NV_ALS2_ADDR;

		return 0;
	}

	return -1;
}

int als_get_sensor_id_by_tag(int32_t tag, uint32_t *s_id)
{
	if (s_id && (tag == TAG_ALS || tag == TAG_ALS1 || tag == TAG_ALS2)) {
		if (tag == TAG_ALS)
			*s_id = ALS;
		else if (tag == TAG_ALS1)
			*s_id = ALS1;
		else if (tag == TAG_ALS2)
			*s_id = ALS2;

		return 0;
	}

	return -1;
}

int als_get_tag_by_sensor_id(uint32_t s_id, int32_t *tag)
{
	if (tag != NULL && (s_id == ALS || s_id == ALS1 || s_id == ALS2)) {
		if (s_id == ALS)
			*tag = TAG_ALS;
		else if (s_id == ALS1)
			*tag = TAG_ALS1;
		else if (s_id == ALS2)
			*tag = TAG_ALS2;

		return 0;
	}

	return -1;
}

static int als_flag_result_cali_and_reset(struct als_device_info *dev_info)
{
	uint32_t t = dev_info->chip_type;

	if ((dev_info->is_cali_supported == 1) ||
		(t == ALS_CHIP_ROHM_RGB) || (t == ALS_CHIP_AVAGO_RGB) ||
		(t == ALS_CHIP_AMS_TMD3725_RGB) || (t == ALS_CHIP_LITEON_LTR582) ||
		(t == ALS_CHIP_APDS9999_RGB) || (t == ALS_CHIP_AMS_TMD3702_RGB) ||
		(t == ALS_CHIP_APDS9253_RGB) || (t == ALS_CHIP_VISHAY_VCNL36658) ||
		(t == ALS_CHIP_AMS_TCS3701_RGB) || (t == ALS_CHIP_AMS_TCS3708_RGB) ||
		(t == ALS_CHIP_AMS_TCS3718_RGB) || (t == ALS_CHIP_AMS_TCS3707_RGB) ||
		(t == ALS_CHIP_BH1749) || (t == ALS_CHIP_BU27006MUC) ||
		(t == ALS_CHIP_AMS_TMD2702_RGB) || (t == ALS_CHIP_AMS_TSL2540_RGB))
		return 1;

	return 0;
}

int als_flag_result_cali(struct als_device_info *dev_info)
{
	if (dev_info && (als_flag_result_cali_and_reset(dev_info) == 1 ||
		(dev_info->chip_type == ALS_CHIP_VD6281)))
		return 1;

	return 0;
}

static int als_flag_result_reset(struct als_device_info *dev_info)
{
	if (dev_info && (als_flag_result_cali_and_reset(dev_info) == 1 ||
		(dev_info->chip_type == ALS_CHIP_TSL2591)))
		return 1;

	return 0;
}

static void reset_calibrate_data_als_ud(int32_t tag,
	struct als_platform_data *pf_data, struct als_device_info *dev_info)
{
	if (pf_data->als_ud_type == ALS_UD_MINUS_DSS_NOISE)
		dev_info->als_rgb_pa_to_sh = 1;
	send_calibrate_data_to_mcu(tag, SUB_CMD_SET_ALS_PA,
		(const void *)&dev_info->als_rgb_pa_to_sh,
		sizeof(dev_info->als_rgb_pa_to_sh), true);
#ifdef CONFIG_CONTEXTHUB_SHMEM
	if (shmem_send(tag, &dev_info->als_under_tp_cal_data,
		als_ud_nv_size(dev_info->als_ud_nv_type)))
		hwlog_err("%s shmem_send error\n", __func__);
#endif
}

static void reset_als_calibrate_data_by_tag(int32_t tag)
{
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;

	pf_data = als_get_platform_data(tag);
	dev_info = als_get_device_info(tag);
	if (!pf_data || !dev_info)
		return;

	if (als_flag_result_reset(dev_info) == 1) {
		if (pf_data->als_phone_type == LAYA) {
			send_calibrate_data_to_mcu(tag, SUB_CMD_SET_OFFSET_REQ,
				dev_info->als_sensor_calibrate_data,
				ALS_CALIDATA_NV_SIZE_WITH_DARK_NOISE_OFFSET, true);
		} else if ((pf_data->als_phone_type == ELLE) ||
			(pf_data->als_phone_type == VOGUE) ||
			(pf_data->als_ud_type == ALS_UD_MINUS_TP_RGB) ||
			(pf_data->als_ud_type == ALS_UD_MINUS_DSS_NOISE)) {
			send_calibrate_data_to_mcu(tag,
				SUB_CMD_SET_OFFSET_REQ,
				dev_info->als_sensor_calibrate_data,
				ALS_CALIDATA_NV_SIZE, true);
			reset_calibrate_data_als_ud(tag, pf_data, dev_info);
		} else {
			send_calibrate_data_to_mcu(tag,
				SUB_CMD_SET_OFFSET_REQ,
				dev_info->als_sensor_calibrate_data,
				ALS_CALIDATA_NV_SIZE, true);
		}
	}
}

void reset_als_calibrate_data(void)
{
	reset_als_calibrate_data_by_tag(TAG_ALS);
}

void reset_als1_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(ALS1)) != 0)
		reset_als_calibrate_data_by_tag(TAG_ALS1);
}

void reset_als2_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(ALS2)) != 0)
		reset_als_calibrate_data_by_tag(TAG_ALS2);
}

void select_als_para_pinhole(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	mutex_lock(&mutex_set_para);
	/*
	 * when the product is Was,
	 * the tp_manufacture is the same as tplcd_manufacture
	 */
	if (pf_data->als_phone_type == WAS)
		tp_manufacture = get_tplcd_manufacture();
	set_pinhole_als_extend_parameters(pf_data, dev_info);
	mutex_unlock(&mutex_set_para);
}

void select_als_para_rpr531(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	mutex_lock(&mutex_set_para);
	set_rpr531_als_extend_prameters(pf_data, dev_info);
	mutex_unlock(&mutex_set_para);
}

void select_als_para_tmd2745(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	mutex_lock(&mutex_set_para);
	set_tmd2745_als_extend_parameters(pf_data, dev_info);
	mutex_unlock(&mutex_set_para);
}

void select_als_para_normal(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	switch (dev_info->chip_type) {
	case ALS_CHIP_VISHAY_VCNL36832:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_vcnl36832_first_table(),
			als_get_vcnl36832_table_count());
	case ALS_CHIP_STK3338:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_stk3338_first_table(),
			als_get_stk3338_table_count());
	case ALS_CHIP_LTR2568:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_ltr2568_first_table(),
			als_get_ltr2568_table_count());
	case ALS_CHIP_VEML32185:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_veml32185_first_table(),
			als_get_veml32185_table_count());
	case ALS_CHIP_STK32670:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_stk32670_first_table(),
			als_get_stk32670_table_count());
	case ALS_CHIP_STK32671:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_stk32671_first_table(),
			als_get_stk32671_table_count());
	case ALS_CHIP_LTR311:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_ltr311_first_table(),
			als_get_ltr311_table_count());
	case ALS_CHIP_SYH399:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_syh399_first_table(),
			als_get_syh399_table_count());
	case ALS_CHIP_STK37670:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_stk37670_first_table(),
			als_get_stk37670_table_count());
	case ALS_CHIP_TSL2585:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_tsl2585_first_table(),
			als_get_tsl2585_table_count());
	case ALS_CHIP_SIP1225:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_sip1225_first_table(),
			als_get_sip1225_table_count());
	case ALS_CHIP_SIP1226:
		return set_als_extend_prameters(pf_data, dev_info,
			als_get_sip1226_first_table(),
			als_get_sip1226_table_count());
	default:
		return;
	}
}

static int select_als_type1_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn)
{
	if (dev_info->chip_type == ALS_CHIP_ROHM_RGB)
		select_rohm_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AVAGO_RGB ||
		dev_info->chip_type == ALS_CHIP_APDS9253_RGB)
		select_avago_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_APDS9999_RGB)
		select_apds9999_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TMD3725_RGB)
		select_ams_tmd3725_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TMD3702_RGB)
		select_ams_tmd3702_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3701_RGB)
		select_ams_tcs3701_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3708_RGB)
		select_ams_tcs3708_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3718_RGB)
		select_ams_tcs3718_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3707_RGB)
		select_ams_tcs3707_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_LTR2594)
		select_liteon_ltr2594_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_STK3638)
		select_stk3638_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_LITEON_LTR582)
		select_liteon_ltr582_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_VISHAY_VCNL36658)
		select_vishay_vcnl36658_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_APDS9922 ||
		dev_info->chip_type == ALS_CHIP_LTR578)
		select_als_para_pinhole(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_RPR531)
		select_als_para_rpr531(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_TMD2745)
		select_als_para_tmd2745(pf_data, dev_info);
	else
		return -1;

	return 0;
}

static bool is_normal(struct als_device_info *dev_info)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(g_als_chip_normal); i++) {
		if (dev_info->chip_type == g_als_chip_normal[i])
			return true;
	}
	return false;
}

static int select_als_type2_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn)
{
	if (dev_info->chip_type == ALS_CHIP_TSL2591)
		set_tsl2591_als_extend_prameters(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_BH1726)
		set_bh1726_als_extend_prameters(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TMD2702_RGB)
		select_ams_tmd2702_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TSL2540_RGB)
		select_ams_tsl2540_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_VD6281)
		set_vd6281_als_extend_prameters(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_BH1749)
		select_rohmbh1749_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_BU27006MUC)
		select_rohmbu27006muc_als_data(pf_data, dev_info);
	else if (is_normal(dev_info))
		select_als_para_normal(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_SIP3623)
		select_sip3623_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_SYN3133)
		select_syn3133_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_SYN3136)
		select_syn3136_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_SIP3642)
		select_sip3642_als_data(pf_data, dev_info);
	else
		return -1;

	return 0;
}

static int select_als_type3_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn)
{
	if (dev_info->chip_type == ALS_CHIP_SY3133_RGB)
		select_silergy_sy3133_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_SIP3510)
		select_sip3510_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_HX32062SE)
		select_hx32062se_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_SIP2326)
		select_sip2326_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_TCS3410)
		select_tcs3410_als_data(pf_data, dev_info);
	else
		return -1;

	return 0;
}

void select_als_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn)
{
	if ((!pf_data) || (!dev_info) || (!dn))
		return;

	if (!select_als_type1_para(pf_data, dev_info, dn)) {
	} else if (!select_als_type2_para(pf_data, dev_info, dn)) {
	} else if (!select_als_type3_para(pf_data, dev_info, dn)) {
	} else if (fill_extend_data_in_dts(dn, "als_extend_data",
		pf_data->als_extend_data, 12, /* max 12 extend data in dts */
		EXTEND_DATA_TYPE_IN_DTS_HALF_WORD) != 0) {
		hwlog_err("als_extend_data:fill_extend_data_in_dts failed\n");
	}
}

void als_on_read_tp_module(unsigned long action, void *data)
{
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;

	pf_data = als_get_platform_data(TAG_ALS);
	dev_info = als_get_device_info(TAG_ALS);
	if (!pf_data || !dev_info)
		return;

	hwlog_info("%s, start\n", __func__);
	if (action == TS_PANEL_UNKNOWN)
		return;
	mutex_lock(&mutex_set_para);
	tp_manufacture = action;
	if ((dev_info->chip_type == ALS_CHIP_APDS9922 ||
		dev_info->chip_type == ALS_CHIP_LTR578) &&
		(dev_info->extend_para_set == 1)) {
		set_pinhole_als_extend_parameters(pf_data, dev_info);
	} else if ((dev_info->chip_type == ALS_CHIP_RPR531) &&
		(dev_info->extend_para_set == 1)) {
		set_rpr531_als_extend_prameters(pf_data, dev_info);
	} else if ((dev_info->chip_type == ALS_CHIP_TMD2745) &&
		(dev_info->extend_para_set == 1)) {
		set_tmd2745_als_extend_parameters(pf_data, dev_info);
	} else {
		hwlog_info("%s, not get sensor yet\n", __func__);
		mutex_unlock(&mutex_set_para);
		return;
	}
	mutex_unlock(&mutex_set_para);
	hwlog_info("%s, get tp module type = %d\n", __func__, tp_manufacture);
	if (dev_info->send_para_flag == 1)
		resend_als_parameters_to_mcu(TAG_ALS);
}
