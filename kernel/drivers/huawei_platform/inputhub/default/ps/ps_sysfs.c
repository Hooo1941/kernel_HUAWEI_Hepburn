/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ps sysfs source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "ps_sysfs.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>

#include "contexthub_route.h"

#ifdef SENSOR_DATA_ACQUISITION
#define PS_TEST_CAL                 "PS"
#define ps_get_digital_offset(x)       (((x) >> 8) & 0x000000FF)
#define ps_get_analog_offset(x)      ((x) & 0x000000FF)
#define PS_CAL_NUM                   8
#define PS_CAL_JOIN_NUM              4
#define PS_THRESHOLD_NUM             8
#define PS_ADD_CAL_NUM               1
#define PS_ADD_THRESHOLD_NUM         1
#define PS_TEST_NAME_MAX_LEN         32
#define PS_DEVICE_NAME_MAX_LEN       12
#define PS_TEST_ADD_LEN              7
#define PS_ALTB_COUNT_SIZE           4
#define PS_CALI_XTALK                703018001
#define PS_FAR_PDATA                 703018002
#define PS_NEAR_PDATA                703018003
#define PS_OFFSET_PDATA              703018004
#define PS_SCREEN_ON_CALI_XTALK      703018005
#define PS_SCREEN_ON_FAR_PDATA       703018006
#define PS_SCREEN_ON_NEAR_PDATA      703018007
#define PS_SCREEN_ON_OFFSET_PDATA    703018008
#define PS_DIGITAL_OFFSET_PDATA      703018015
#endif

#ifdef SENSOR_DATA_ACQUISITION
static char *ps_test_name[PS_CAL_NUM] = {
	"PS_XTALK", "PS_FAR",
	"PS_NEAR", "PS_OFFSET",
	"PS_SCREEN_ON_CALI_XTALK_MSG", "PS_SCREEN_ON_FAR_PDATA_MSG",
	"PS_SCREEN_ON_NEAR_PDATA_MSG", "PS_SCREEN_ON_OFFSET_PDATA_MSG"
};
static char *ps_add_test_name = "PS_DIGITAL_OFFSET_MSG";
static char ps_test_with_device_name[PS_CAL_NUM][PS_TEST_NAME_MAX_LEN];
static bool ps_test_name_joined_flag;
#endif

static int g_stop_auto_ps;

int get_stop_auto_ps(void)
{
	return g_stop_auto_ps;
}

#ifdef SENSOR_DATA_ACQUISITION
/* add ps special data to factory big data */
static void ps_add_do_enq_work(struct ps_device_info *dev_info,
	int32_t *ps_digital_offset)
{
	struct ps_platform_data *pf_data = NULL;
	int32_t min_threshold = 0;
	int32_t max_threshold;
	struct sensor_eng_cal_test ps_test = { 0 };

	pf_data = ps_get_platform_data(TAG_PS);
	if (pf_data == NULL)
		return;
	max_threshold = (int32_t)pf_data->digital_offset_max;

	ps_test.cal_value = ps_digital_offset;
	ps_test.value_num = PS_ADD_CAL_NUM;
	ps_test.threshold_num = PS_ADD_THRESHOLD_NUM;
	ps_test.first_item = PS_DIGITAL_OFFSET_PDATA;
	ps_test.min_threshold = &min_threshold;
	ps_test.max_threshold = &max_threshold;
	if (memcpy_s(ps_test.name, sizeof(ps_test.name),
		PS_TEST_CAL, sizeof(PS_TEST_CAL)) != EOK)
		return;
	if (memcpy_s(ps_test.result, sizeof(ps_test.result),
		sensor_cal_result(dev_info->ps_calibration_res),
		(strlen(sensor_cal_result(dev_info->ps_calibration_res)) +
		1)) != EOK)
		return;
	ps_test.test_name[0] = ps_add_test_name;
	enq_notify_work_sensor(ps_test);
}
#endif

ssize_t attr_ps_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ps_device_info *dev_info = NULL;

	dev_info = ps_get_device_info(TAG_PS);
	if (dev_info == NULL)
		return -1;
	hwlog_info("attr_ps_calibrate_show result=%d\n",
		(int32_t)dev_info->ps_calibration_res);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		(int32_t)dev_info->ps_calibration_res);
}

/* temp: buffer to write, length: bytes to write */
int write_ps_offset_to_nv(int *temp, uint16_t length)
{
	int ret = 0;
	const int *poffset_data = (const int *)user_info.nv_data;

	if (!temp) {
		hwlog_err("write_ps_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (length > PS_CALIDATA_NV_SIZE)
		return -1;

	if (write_calibrate_data_to_nv(PS_CALIDATA_NV_NUM, length,
		"PSENSOR", (char *)temp))
		return -1;

	hwlog_info("write_ps_offset_to_nv temp: %d,%d,%d\n",
		temp[0], temp[1], temp[2]);
	if (memcpy_s(&ps_sensor_calibrate_data,
		sizeof(ps_sensor_calibrate_data),
		temp, length) != EOK)
		return -1;
	hwlog_info("nve_direct_access write temp %d,%d,%d\n",
		*poffset_data, *(poffset_data + 1), *(poffset_data + 2));
	msleep(10); /* sleep 10 ms */
	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		PS_CALIDATA_NV_SIZE, "PSENSOR"))
		return -1;

	if ((*poffset_data != temp[0]) || (*(poffset_data+1) != temp[1]) ||
	    (*(poffset_data + 2) != temp[2])) {
		hwlog_err("nv write fail, %d,%d,%d\n",
			*poffset_data, *(poffset_data + 1), *(poffset_data + 2));
		return -1;
	}
	return ret;
}

static void ps_calibrate_xtalk(struct ps_device_info *dev_info,
	unsigned long val, const struct read_info *p)
{
	uint32_t temp;
	int16_t ps_calibate_offset_0;
	int16_t ps_calibate_offset_3;

	ps_sensor_offset[val - 1] = *((int32_t *)p->data);
	temp = (uint32_t)ps_sensor_offset[val - 1];
	temp = (temp & 0x0000ffff);
	ps_calibate_offset_0 = (int16_t)temp;
	temp = (uint32_t)ps_sensor_offset[val - 1];
	temp = ((temp & 0xffff0000) >> 16);
	ps_calibate_offset_3 = (int16_t)temp;
	dev_info->ps_calib_data_for_data_collect[0] =
		(int32_t)ps_calibate_offset_0;
	dev_info->ps_calib_data_for_data_collect[3] =
		(int32_t)ps_calibate_offset_3;
	/* clear NV 5cm calibrated value */
	ps_sensor_offset[1] = 0;
	/* clear NV 3cm calibrated value */
	ps_sensor_offset[2] = 0;
	hwlog_info("ps calibrate success, ps_calibate_offset_0=%d, ps_calibate_offset_3=%d\n",
		(int32_t)ps_calibate_offset_0, (int32_t)ps_calibate_offset_3);
	hwlog_info("ps calibrate success, data=%d, len=%d val=%lu\n",
		ps_sensor_offset[val - 1], p->data_length, val);
}

static void ps_calibrate_save_minpdata(struct ps_device_info *dev_info,
	unsigned long val,struct read_info *p)
{
	ps_sensor_offset[0] = *((int32_t *)p->data);
	dev_info->ps_calib_data_for_data_collect[0] = ps_sensor_offset[0];
	hwlog_info("ps calibrate success, data=%d, len=%d val=%lu\n",
		ps_sensor_offset[0], p->data_length, val);
}

static void ps_calibrate_screen_on_xtalk(struct ps_device_info *dev_info,
	unsigned long val, struct read_info *p)
{
	uint32_t temp;
	int16_t ps_calibate_offset_4;
	int16_t ps_calibate_offset_7;

	ps_sensor_offset[3] = *((int32_t *)p->data);
	temp = (uint32_t)ps_sensor_offset[3];
	temp = (temp & 0x0000ffff);
	ps_calibate_offset_4 = (int16_t)temp;
	temp = (uint32_t)ps_sensor_offset[3];
	temp = ((temp & 0xffff0000) >> 16);
	ps_calibate_offset_7 = (int16_t)temp;
	dev_info->ps_calib_data_for_data_collect[4] =
		(int32_t)ps_calibate_offset_4;
	dev_info->ps_calib_data_for_data_collect[7] =
		(int32_t)ps_calibate_offset_7;
	/* clear NV 5cm screen_on calibrated value */
	ps_sensor_offset[4] = 0;
	/* clear NV 3cm screen_on calibrated value */
	ps_sensor_offset[5] = 0;
	hwlog_info("ps calibrate success, data=%d, len=%d val=%lu\n",
		ps_sensor_offset[3], p->data_length, val);
}

static void ps_calibrate_screen_on_5cm(struct ps_device_info *dev_info,
	unsigned long val, struct read_info *p)
{
	ps_sensor_offset[4] = *((int32_t *)p->data);
	dev_info->ps_calib_data_for_data_collect[5] = *((int32_t *)p->data);
	hwlog_info("ps calibrate success, data=%d, len=%d val=%lu\n",
		ps_sensor_offset[4], p->data_length, val);
}

static void ps_calibrate_screen_on_3cm(struct ps_device_info *dev_info,
	unsigned long val, struct read_info *p)
{
	ps_sensor_offset[5] = *((int32_t *)p->data);
	dev_info->ps_calib_data_for_data_collect[6] = *((int32_t *)p->data);
	hwlog_info("ps calibrate success, data=%d, len=%d val=%lu\n",
		ps_sensor_offset[5], p->data_length, val);
}

static void ps_calibrate_not_minpdata_mode(struct ps_device_info *dev_info,
	unsigned long val, struct read_info *p)
{
	uint32_t temp;
	int16_t ps_calibate_offset_0;
	int16_t ps_calibate_offset_3;

	temp = (uint32_t)ps_sensor_offset[0];
	temp = (temp & 0x0000ffff);
	ps_calibate_offset_0 = (int16_t)temp;
	temp = (uint32_t)ps_sensor_offset[0];
	temp = ((temp & 0xffff0000) >> 16);
	ps_calibate_offset_3 = (int16_t)temp;
	dev_info->ps_calib_data_for_data_collect[0] =
		(int32_t)ps_calibate_offset_0;
	dev_info->ps_calib_data_for_data_collect[3] =
		(int32_t)ps_calibate_offset_3;
	ps_sensor_offset[val - 1] = *((int32_t *)p->data);
	dev_info->ps_calib_data_for_data_collect[val - 1] =
		*((int32_t *)p->data);
	hwlog_info("ps calibrate success, ps_calibate_offset_0=%d, ps_calibate_offset_3=%d\n",
		(int32_t)ps_calibate_offset_0, (int32_t)ps_calibate_offset_3);
	hwlog_info("ps calibrate success, data=%d, len=%d val=%lu\n",
		ps_sensor_offset[val-1], p->data_length, val);
}

static void ps_calibrate_process(struct ps_device_info *dev_info,
	unsigned long val, struct read_info *pkg_mcu)
{
	struct read_info *p = pkg_mcu;

	if (!p) {
		hwlog_info("ps calibrate process error, pkg is null\n");
		return;
	}

	if (val == PS_XTALK_CALIBRATE)
		ps_calibrate_xtalk(dev_info, val, p);
	else if (val == PS_SAVE_MINPDATA)
		ps_calibrate_save_minpdata(dev_info, val, p);
	else if (val == PS_SCREEN_ON_XTALK_CALIBRATE)
		ps_calibrate_screen_on_xtalk(dev_info, val, p);
	else if (val == PS_SCREEN_ON_5CM_CALIBRATE)
		ps_calibrate_screen_on_5cm(dev_info, val, p);
	else if (val == PS_SCREEN_ON_3CM_CALIBRATE)
		ps_calibrate_screen_on_3cm(dev_info, val, p);
	else if (val != PS_MINPDATA_MODE)
		ps_calibrate_not_minpdata_mode(dev_info, val, p);
}

static void ps_calibrate_process_ext(struct ps_device_info *dev_info,
	unsigned long val, int32_t *ps_cali_data, int length)
{
	uint8_t shift_offset = 0;

	if (!ps_cali_data || length <= 0) {
		hwlog_err("%s invalid argument", __func__);
		return;
	}

	if (memcpy_s(ps_sensor_offset, sizeof(ps_sensor_offset),
		ps_cali_data, length) != EOK) {
		hwlog_err("%s:memcpy ps_sensor_offset fail\n", __func__);
		return;
	}
	if (ps_cali_data[PS_CALIBRATE_DATA_LENGTH - 1] == PS_PULSE_LEN_64)
		shift_offset = PS_BIG_DATA_OFFSET;

	if (memcpy_s(dev_info->ps_calib_data_for_data_collect,
		sizeof(dev_info->ps_calib_data_for_data_collect),
		ps_cali_data + shift_offset,
		sizeof(dev_info->ps_calib_data_for_data_collect)) != EOK) {
		hwlog_err("%s:memcpy data_collect fail\n", __func__);
		return;
	}

	for (int i = PS_CALIB_COLLECT_4; i <= PS_CALIB_COLLECT_7; i++)
		dev_info->ps_calib_data_for_data_collect[i] = 0;

	hwlog_info("ps calibrate success, ps_cali_data %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
		ps_cali_data[PS_XTALK0], ps_cali_data[PS_5CM_PDATA0],
		ps_cali_data[PS_3CM_PDATA0], ps_cali_data[PS_OFFSET_NEW0],
		ps_cali_data[PS_OFFSET_OLD0], ps_cali_data[PS_XTALK_OLD0],
		ps_cali_data[PS_XTALK1], ps_cali_data[PS_5CM_PDATA1],
		ps_cali_data[PS_3CM_PDATA1], ps_cali_data[PS_OFFSET_NEW1],
		ps_cali_data[PS_OFFSET_OLD1], ps_cali_data[PS_XTALK_OLD1],
		ps_cali_data[PS_PULSE_LEN]);
}

static int ps_calibrate_save(const void *buf, int length)
{
	struct ps_device_info *dev_info = NULL;
	int32_t temp_buf[PS_CALIBRATE_DATA_LENGTH] = { 0 };
	int ret;

	dev_info = ps_get_device_info(TAG_PS);
	if (dev_info == NULL)
		return -1;
	if (!buf || length <= 0 || length > sizeof(ps_sensor_offset)) {
		hwlog_err("%s invalid argument", __func__);
		dev_info->ps_calibration_res = EXEC_FAIL;
		return -1;
	}
	if (memcpy_s(temp_buf, sizeof(temp_buf), buf, length) != EOK)
		return -1;
	hwlog_info("%s:psensor calibrate ok, %d,%d,%d,%d,%d,%d\n", __func__,
		temp_buf[0], temp_buf[1], temp_buf[2],
		temp_buf[3], temp_buf[4], temp_buf[5]);
	ret = write_ps_offset_to_nv(temp_buf, sizeof(temp_buf));
	if (ret) {
		ret = mcu_save_calidata_to_nv(TAG_PS, temp_buf);
		if (ret != 0) {
			hwlog_err("nv write fail\n");
			dev_info->ps_calibration_res = NV_FAIL;
			return -1;
		}
	}
	dev_info->ps_calibration_res = SUC;
	return 0;
}

static int ps1_calibrate_save(const void *buf, int length)
{
	struct ps_device_info *dev_info = NULL;
	int32_t temp_buf[PS_CALIBRATE_DATA_LENGTH] = { 0 };
	int ret;

	dev_info = ps_get_device_info(TAG_PS1);
	if (dev_info == NULL)
		return -1;
	if (!buf || length <= 0 || length > sizeof(ps_sensor_offset)) {
		hwlog_err("%s invalid argument", __func__);
		dev_info->ps_calibration_res = EXEC_FAIL;
		return -1;
	}
	if (memcpy_s(temp_buf, sizeof(temp_buf), buf, length) != EOK)
		return -1;
	hwlog_info("%s:psensor calibrate ok, %d,%d,%d,%d,%d,%d\n", __func__,
		temp_buf[0], temp_buf[1], temp_buf[2],
		temp_buf[3], temp_buf[4], temp_buf[5]);
	ret = write_ps_offset_to_nv(temp_buf, sizeof(temp_buf));
	if (ret) {
		ret = mcu_save_calidata_to_nv(TAG_PS1, temp_buf);
		if (ret != 0) {
			hwlog_err("nv write fail\n");
			dev_info->ps_calibration_res = NV_FAIL;
			return -1;
		}
	}
	dev_info->ps_calibration_res = SUC;
	return 0;
}

#ifdef SENSOR_DATA_ACQUISITION
static const char *ps_get_lcd_type(void)
{
	const char *lcd_model = NULL;
	const char *sec_lcd_model = NULL;
	const char *target_model = NULL;
	struct ps_platform_data *pf_data = NULL;
	struct device_node *np = NULL;

	pf_data = ps_get_platform_data(TAG_PS);
	if (!pf_data) {
		hwlog_err("%s: ps under tp no tag\n", __func__);
		return target_model;
	}
	hwlog_info("%s ps_phone_type = %d\n", __func__, pf_data->oily_count_size);

	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("ps not find lcd_panel_type node\n");
		return target_model;
	}

	if (of_property_read_string(np, "lcd_panel_type", &lcd_model))
		hwlog_err("ps not find lcd_panel_type in dts\n");
	if (of_property_read_string(np, "sec_lcd_panel_type", &sec_lcd_model))
		hwlog_info("ps not find sec_lcd_panel_type in dts\n");
	hwlog_info("%s lcd_model %s, sec_lcd_model %s\n", __func__, lcd_model, sec_lcd_model);

	target_model = lcd_model;
	if (pf_data->oily_count_size == PS_ALTB_COUNT_SIZE)
		target_model = sec_lcd_model;
	hwlog_info("%s lcd_panel_type: %s suc in dts!!\n", __func__, target_model);
	return target_model;
}

static void ps_test_name_join_with_device(enum sensor_detect_list index)
{
	int i;
	int ret;
	char device_name[PS_DEVICE_NAME_MAX_LEN] = {0};
	char lcd_name[PS_TEST_NAME_MAX_LEN] = "_L_";
	struct sensorlist_info *sensorlist_info_ps =
		get_sensorlist_info_by_index(index);

	const char *lcd_model = ps_get_lcd_type();
	if (lcd_model != NULL) {
		ret = strncat_s(lcd_name, sizeof(lcd_name), lcd_model, PS_TEST_ADD_LEN);
		if (ret)
			hwlog_err("%s ps memcpy lcdmodel fail\n", __func__);
	}

	for (i = 0; i < PS_CAL_NUM; i++) {
		ret = strcpy_s(ps_test_with_device_name[i],
			strlen(ps_test_name[i]) + 1, ps_test_name[i]);
		if (ret)
			hwlog_err("%s ps test name copy fail %d %d\n", __func__, i, ret);
	}

	for (i = 0; i < PS_CAL_JOIN_NUM; i++) {
		ret = strcat_s(ps_test_with_device_name[i], sizeof(ps_test_with_device_name), "_");
		if (ret)
			hwlog_err("%s strcat fail %d\n", __func__, i);

		ret = strncat_s(ps_test_with_device_name[i],
			sizeof(ps_test_with_device_name),
			sensorlist_info_ps->name,
			strlen(sensorlist_info_ps->name));
		if (ret)
			hwlog_err("%s ps_name strncat fail %d %d\n", __func__, i, ret);
		ret = strncat_s(ps_test_with_device_name[i],
			sizeof(ps_test_with_device_name), lcd_name, strlen(lcd_name));
		if (ret)
			hwlog_err("%s ps strncat fail dev id %d, ret %d\n", __func__, i, ret);
		hwlog_info("%s : %s\n", __func__, ps_test_with_device_name[i]);
	}

	ps_test_name_joined_flag = true;
}
#endif /* SENSOR_DATA_ACQUISITION */

static void do_ps_calibrate(unsigned long val)
{
	struct ps_device_info *dev_info = NULL;
	enum ret_type rtype = RET_INIT;
	struct read_info pkg_mcu;

#ifdef SENSOR_DATA_ACQUISITION
	if (ps_test_name_joined_flag == false)
		ps_test_name_join_with_device(PS);
#endif /* SENSOR_DATA_ACQUISITION */
	dev_info = ps_get_device_info(TAG_PS);
	if (dev_info == NULL)
		return;
	pkg_mcu = send_calibrate_cmd(TAG_PS, val, &rtype);
	dev_info->ps_calibration_res = rtype;
	/* COMMU_FAIL=4	EXEC_FAIL=2 */
	if (dev_info->ps_calibration_res == COMMU_FAIL ||
	    dev_info->ps_calibration_res == EXEC_FAIL) {
		return;
	} else if (pkg_mcu.errno == 0) {
		if (dev_info->update_ps_cali == 0)
			ps_calibrate_process(dev_info, val, &pkg_mcu);
		else
			ps_calibrate_process_ext(dev_info, val,
			(int32_t *)pkg_mcu.data, pkg_mcu.data_length);
	}
	if ((val == PS_5CM_CALIBRATE) || (val == PS_MINPDATA_MODE) ||
	    (val == PS_SCREEN_ON_5CM_CALIBRATE))
		dev_info->ps_calibration_res = SUC;
	else
		ps_calibrate_save(ps_sensor_offset, sizeof(ps_sensor_offset));
}

static void do_ps1_calibrate(unsigned long val)
{
	struct ps_device_info *dev_info = NULL;
	enum ret_type rtype = RET_INIT;
	struct read_info pkg_mcu;

#ifdef SENSOR_DATA_ACQUISITION
	if (ps_test_name_joined_flag == false)
		ps_test_name_join_with_device(PS1);
#endif /* SENSOR_DATA_ACQUISITION */
	dev_info = ps_get_device_info(TAG_PS1);
	if (dev_info == NULL)
		return;
	pkg_mcu = send_calibrate_cmd(TAG_PS1, val, &rtype);
	dev_info->ps_calibration_res = rtype;
	/* COMMU_FAIL=4	EXEC_FAIL=2 */
	if (dev_info->ps_calibration_res == COMMU_FAIL ||
		dev_info->ps_calibration_res == EXEC_FAIL) {
		return;
	} else if (pkg_mcu.errno == 0) {
		if (dev_info->update_ps_cali == 0)
			ps_calibrate_process(dev_info, val, &pkg_mcu);
		else
			ps_calibrate_process_ext(dev_info, val,
			(int32_t *)pkg_mcu.data, pkg_mcu.data_length);
	}
	if ((val == PS_5CM_CALIBRATE) || (val == PS_MINPDATA_MODE) ||
		(val == PS_SCREEN_ON_5CM_CALIBRATE))
		dev_info->ps_calibration_res = SUC;
	else
		ps1_calibrate_save(ps_sensor_offset, sizeof(ps_sensor_offset));
}

#ifdef SENSOR_DATA_ACQUISITION
static void ps_calibrate_enq_notify_work(struct ps_platform_data *pf_data,
	struct ps_device_info *dev_info, unsigned long val)
{
	int32_t *ps_cali_data = dev_info->ps_calib_data_for_data_collect;
	struct sensor_eng_cal_test ps_test = { 0 };
	int32_t i;
	int32_t digital_offset;
	int32_t min_threshold_ps[PS_CAL_NUM] = { 0, 0, 0, -pf_data->offset_min,
		0, 0, 0, -pf_data->offset_min };
	int32_t max_threshold_ps[PS_CAL_NUM] = {
		pf_data->ps_calib_20cm_threshold,
		pf_data->ps_calib_5cm_threshold,
		pf_data->ps_calib_3cm_threshold, pf_data->offset_max,
		pf_data->ps_calib_20cm_threshold,
		pf_data->ps_calib_5cm_threshold,
		pf_data->ps_calib_3cm_threshold, pf_data->offset_max
	};

	if (get_sensor_tof_flag() == 1) {
		min_threshold_ps[0] = 0;
		min_threshold_ps[1] = 0;
		min_threshold_ps[2] = 0;
		min_threshold_ps[3] = 0;
		max_threshold_ps[0] = tof_data.tof_calib_zero_threshold;
		max_threshold_ps[1] = tof_data.tof_calib_6cm_threshold;
		max_threshold_ps[2] = tof_data.tof_calib_10cm_threshold;
		max_threshold_ps[3] = tof_data.tof_calib_60cm_threshold;
	}

	hwlog_info("%s:is sport data collect\n", __func__);
	if (dev_info->chip_type == PS_CHIP_APDS9253_006) {
		digital_offset = ps_get_digital_offset(ps_cali_data[3]);
		ps_cali_data[3] = ps_get_analog_offset(ps_cali_data[3]);
		ps_add_do_enq_work(dev_info, &digital_offset);
		msleep(5); /* sleep 5 ms */
	}
	if (val == PS_5CM_CALIBRATE || val == PS_SCREEN_ON_XTALK_CALIBRATE)
		return;
	ps_test.cal_value = ps_cali_data;
	ps_test.first_item = PS_CALI_XTALK;
	ps_test.value_num = PS_CAL_NUM;
	ps_test.threshold_num = PS_THRESHOLD_NUM;
	ps_test.min_threshold = min_threshold_ps;
	ps_test.max_threshold = max_threshold_ps;
	if (memcpy_s(ps_test.name, sizeof(ps_test.name),
		PS_TEST_CAL, sizeof(PS_TEST_CAL)) != EOK)
		return;
	if (memcpy_s(ps_test.result, sizeof(ps_test.result),
		sensor_cal_result(dev_info->ps_calibration_res),
		(strlen(sensor_cal_result(dev_info->ps_calibration_res)) +
		1)) != EOK)
		return;
	for (i = 0; i < PS_CAL_NUM; i++)
		ps_test.test_name[i] = ps_test_with_device_name[i];

	enq_notify_work_sensor(ps_test);
}
#endif

ssize_t attr_ps_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct ps_platform_data *pf_data = NULL;
	struct ps_device_info *dev_info = NULL;
	unsigned long val = 0;

	pf_data = ps_get_platform_data(TAG_PS);
	dev_info = ps_get_device_info(TAG_PS);
	if (pf_data == NULL || dev_info == NULL)
		return -1;

#ifdef SENSOR_DATA_ACQUISITION
	if (get_sensor_tof_flag() == 1)
		hwlog_info("tof sensor maxThreshold %d %d %d %d\n",
			tof_data.tof_calib_zero_threshold,
			tof_data.tof_calib_6cm_threshold,
			tof_data.tof_calib_10cm_threshold,
			tof_data.tof_calib_60cm_threshold);
	hwlog_info("ps sensor offset_min=%d,offset_max=%d\n",
		-pf_data->offset_min, pf_data->offset_max);
#endif

	if ((get_sensor_tof_flag() != 1) &&
		(dev_info->chip_type == PS_CHIP_NONE) &&
		(dev_info->ps_cali_supported != 1)) {
		hwlog_info("%s:no need calibrate chip_type=%d ps_cali_supported=%d\n",
			__func__, dev_info->chip_type,
			dev_info->ps_cali_supported);
		return count;
	}

	if (kstrtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	hwlog_info("ps or tof calibrate order is %lu\n", val);
	if ((val < PS_XTALK_CALIBRATE) || (val > PS_SCREEN_ON_3CM_CALIBRATE)) {
		hwlog_err("set ps or tof calibrate val invalid,val=%lu\n", val);
		dev_info->ps_calibration_res = EXEC_FAIL;
		return count;
	}
	/* ps calibrate */
	if (((val >= PS_XTALK_CALIBRATE && val <= PS_3CM_CALIBRATE) ||
		val == PS_MINPDATA_MODE || val == PS_SAVE_MINPDATA ||
		val >= PS_SCREEN_ON_XTALK_CALIBRATE) &&
		(get_sensor_tof_flag() != 1)) {
		do_ps_calibrate(val);
	} else {
		/* tof calibrate */
		do_tof_calibrate(val);
	}

#ifdef SENSOR_DATA_ACQUISITION
	ps_calibrate_enq_notify_work(pf_data, dev_info, val);
#endif

	return count;
}

ssize_t attr_ps1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct ps_platform_data *pf_data = NULL;
	struct ps_device_info *dev_info = NULL;
	unsigned long val = 0;

	pf_data = ps_get_platform_data(TAG_PS1);
	dev_info = ps_get_device_info(TAG_PS1);
	if (pf_data == NULL || dev_info == NULL)
		return -1;

	if ((get_sensor_tof_flag() != 1) &&
		(dev_info->chip_type == PS_CHIP_NONE) &&
		(dev_info->ps_cali_supported != 1)) {
		hwlog_info("%s:no need calibrate chip_type=%d ps_cali_supported=%d\n",
			__func__, dev_info->chip_type,
			dev_info->ps_cali_supported);
		return count;
	}

	if (kstrtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	hwlog_info("ps or tof calibrate order is %lu\n", val);
	if ((val < PS_XTALK_CALIBRATE) || (val > PS_SCREEN_ON_3CM_CALIBRATE)) {
		hwlog_err("set ps or tof calibrate val invalid,val=%lu\n", val);
		dev_info->ps_calibration_res = EXEC_FAIL;
		return count;
	}
	/* ps calibrate */
	if (((val >= PS_XTALK_CALIBRATE && val <= PS_3CM_CALIBRATE) ||
		val == PS_MINPDATA_MODE || val == PS_SAVE_MINPDATA ||
		val >= PS_SCREEN_ON_XTALK_CALIBRATE) &&
		(get_sensor_tof_flag() != 1)) {
		do_ps1_calibrate(val);
	} else {
		/* tof calibrate */
		do_tof_calibrate(val);
	}

#ifdef SENSOR_DATA_ACQUISITION
	ps_calibrate_enq_notify_work(pf_data, dev_info, val);
#endif

	return count;
}

ssize_t attr_set_stop_ps_auto_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	g_stop_auto_ps = (int)simple_strtoul(buf, NULL, TO_HEXADECIMAL);
	hwlog_err("%s stop_auto_ps %d\n", __func__, g_stop_auto_ps);
	return size;
}

ssize_t attr_set_dt_ps_sensor_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_data event;
	unsigned long source;

	source = simple_strtoul(buf, NULL, TO_HEXADECIMAL);
	hwlog_err("%s buf %s, source %lu\n", __func__, buf, source);

	event.type = TAG_PS;
	event.length = 4; /* 4 bytes */
	event.value[0] = (int)source;

	inputhub_route_write(ROUTE_SHB_PORT, (char *)&event, 8); /* 8 bytes */
	return size;
}

ssize_t attr_get_ps_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char *id_string = NULL;

	id_string = ps_get_sensors_id_string();
	if (id_string == NULL)
		return -1;

	hwlog_info("%s is %s\n", __func__, id_string);
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%s\n",
		id_string);
}

ssize_t sensors_calibrate_show_ps(int tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ps_device_info *dev_info = NULL;

	dev_info = ps_get_device_info(tag);
	if (dev_info == NULL)
		return -1;
	hwlog_info("feima sensors_calibrate_show res=%d\n",
		(int32_t)(dev_info->ps_calibration_res));
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		(int32_t)(dev_info->ps_calibration_res != SUC));
}

