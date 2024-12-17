/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2016. All rights reserved.
 * Description: dev drvier to communicate with sensorhub common
 * Create: 2011-04-11
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/thermal.h>

#include "sensor_commom.h"
#include "../../clt/kernel_clt_flag.h"
#include "securec.h"
#include "sensor_power.h"

//lint -save -e529 -e542
static int is_fpga = 0; // default is no fpga

static int hw_sensor_power_control(sensor_t *s_ctrl, bool on)
{
	int ret = 0;
	if (!s_ctrl) {
		cam_err("%s s_ctrl is null", __func__);
		return -1;
	}

	if (kernel_is_clt_flag()) {
		cam_debug("%s just return for CLT camera", __func__);
		return 0;
	}
	/* fpga board compatibility */
	if (hw_is_fpga_board()) {
		cam_info("%s just return for fpga board", __func__);
		return 0;
	}

	if (on) {
#ifdef CONFIG_KERNEL_CAMERA_USE_SINGLEFRAME
		ret = sensor_power_on_off_by_type(s_ctrl->power_group, on, PS_PUBLIC);
#endif
		ret += sensor_enable_power_setting(s_ctrl->power_group);
		return ret;
	}
#ifdef CONFIG_KERNEL_CAMERA_USE_SINGLEFRAME
	sensor_power_on_off_by_type(s_ctrl->power_group, on, PS_PUBLIC);
#endif
	sensor_disable_power_setting(s_ctrl->power_group);
	return 0;
}

int hw_sensor_power_up(sensor_t *s_ctrl)
{
	return hw_sensor_power_control(s_ctrl, true);
}

int hw_sensor_power_down(sensor_t *s_ctrl)
{
	return hw_sensor_power_control(s_ctrl, false);
}

static int dt_read_u32_array(struct device_node *np, const char *prop,
			     uint32_t *data, uint32_t count)
{
	int sz;

	sz = of_property_count_elems_of_size(np, prop, sizeof(u32));
	if (sz < count) {
		cam_err("%pOF prop:%s elems:%d need:%u", np, prop, sz, count);
		return -ENODATA;
	}
	if (sz > count)
		cam_warn("%pOF prop %s elems:%d need:%u", np, prop, sz, count);

	return of_property_read_u32_array(np, prop, data, count);
}

int hw_sensor_get_phyinfo_data(struct device_node *np,
			       hwsensor_board_info_t *binfo,
			       int info_count)
{
	int ret = 0;
	phy_info_t *phyinfo;

	if (!np || !binfo) {
		cam_err("%s param is invalid", __func__);
		return -1;
	}
	phyinfo = &binfo->phyinfo;

	ret = dt_read_u32_array(np, "vendor,is_master_sensor",
				(u32 *)&phyinfo->is_master_sensor, info_count);
	if (ret)
		return ret;

	ret = dt_read_u32_array(np, "vendor,phy_id", (u32 *)&phyinfo->phy_id,
				info_count);
	if (ret)
		return ret;

	ret = dt_read_u32_array(np, "vendor,phy_mode",
				(u32 *)&phyinfo->phy_mode, info_count);
	if (ret)
		return ret;

	ret = dt_read_u32_array(np, "vendor,phy_freq_mode",
				(u32 *)&phyinfo->phy_freq_mode, info_count);
	if (ret)
		return ret;

	ret = dt_read_u32_array(np, "vendor,phy_freq",
				(u32 *)&phyinfo->phy_freq, info_count);
	if (ret)
		return ret;

	ret = dt_read_u32_array(np, "vendor,phy_work_mode",
				(u32 *)&phyinfo->phy_work_mode, info_count);
	if (ret)
		return ret;

	cam_info("%s, info_count = %d\n"
		 "is_master_sensor[0] = %d, is_master_sensor[1] = %d\n"
		 "phy_id[0] = %d, phy_id[1] = %d\n"
		 "phy_mode[0] = %d, phy_mode[1] = %d\n"
		 "phy_freq_mode[0] = %d, phy_freq_mode[1] = %d\n"
		 "phy_freq[0] = %d, phy_freq[1] = %d\n"
		 "phy_work_mode[0] = %d, phy_work_mode[1] = %d",
		 __func__, info_count, phyinfo->is_master_sensor[0],
		 phyinfo->is_master_sensor[1], phyinfo->phy_id[0],
		 phyinfo->phy_id[1], phyinfo->phy_mode[0], phyinfo->phy_mode[1],
		 phyinfo->phy_freq_mode[0], phyinfo->phy_freq_mode[1],
		 phyinfo->phy_freq[0], phyinfo->phy_freq[1],
		 phyinfo->phy_work_mode[0], phyinfo->phy_work_mode[1]);

	return 0;
}

/* dt_read_u32 - for optional property, no redundent logs */
static inline void dt_read_u32(struct device_node *np, const char *prop,
			       uint32_t *data)
{
	if (of_property_read_u32(np, prop, data) < 0)
		cam_debug("%pOF read %s failed, rc:%d, default value %#x", np,
			  prop, *data);
	else
		cam_debug("%pOF read %s succ, value %#x", np, prop, *data);
}

/* dt_read_u32_essential - log if read succ or fail */
static inline int dt_read_u32_essential(struct device_node *np,
					const char *prop, uint32_t *data)
{
	int rc;

	rc = of_property_read_u32(np, prop, data);
	if (rc < 0)
		cam_err("%pOF read %s failed, rc:%d, default value %#x", np,
			prop, *data);
	else
		cam_info("%pOF read %s succ, value %#x", np, prop, *data);
	return rc;
}

/* hw_sensor_get_ext_name - return ext_name count read. */
static int hw_sensor_get_ext_name(struct device_node *np,
				  hwsensor_board_info_t *binfo)
{
	int i;
	int count;
	const char *ext_name[EXT_NAME_NUM] = { 0 };

	count = of_property_count_strings(np, "vendor,ext_name");
	if (count <= 0 || count > EXT_NAME_NUM) {
		cam_err("%s ext name num = %d out of range", __func__, count);
		return 0;
	}

	(void)of_property_read_string_array(np, "vendor,ext_name", ext_name,
					    count);
	for (i = 0; i < count; i++) {
		if (strcpy_s(binfo->ext_name[i], DEVICE_NAME_SIZE,
			     ext_name[i]) != 0)
			cam_err("%s ext_name copy error", __func__);
		else
			cam_info("%s index:%d ext_name: %s", __func__, i,
				 ext_name[i]);
	}
	return count;
}

static int hw_sensor_get_ext_adc_channel(struct device_node *np,
					 hwsensor_board_info_t *binfo)
{
	int count;

	binfo->adc_channel = -1;
	if (dt_read_u32_essential(np, "vendor,adc_channel",
				  (u32 *)&binfo->adc_channel) < 0)
		return 0;

	count = of_property_count_u32_elems(np, "vendor,adc_threshold");
	if (count <= 0 || count > EXT_THRESHOLD_NUM) {
		cam_err("%s ext threshold num = %d out of range", __func__,
			count);
		return 0;
	}
	(void)dt_read_u32_array(np, "vendor,adc_threshold",
				(u32 *)binfo->adc_threshold, count);
	return count;
}

static int hw_sensor_get_ext_adc_gpio(struct device_node *np,
				      hwsensor_board_info_t *binfo)
{
	int i;
	int count;

	count = of_property_count_u32_elems(np, "vendor,adc_gpio");
	if (count <= 0 || count > ADC_GPIO_NUM_MAX) {
		cam_err("%s ext adb_gpio num = %d out of range", __func__,
			count);
		return 0;
	}
	(void)dt_read_u32_array(np, "vendor,adc_gpio", (u32 *)binfo->adc_gpio,
				count);
	for (i = 0; i < count; i++)
		cam_info("%s binfo->adc_gpio[%d] is %d", __func__, i,
			 binfo->adc_gpio[i]);
	return count;
}

static void hw_sensor_get_ext_dt_data(struct device_node *np,
				      hwsensor_board_info_t *binfo)
{
	binfo->ext_type = NO_EXT_INFO;
	if (dt_read_u32_essential(np, "vendor,ext_type",
				  (u32 *)&binfo->ext_type) < 0)
		return;

	// normally use ext_name count as ext_num
	binfo->ext_num = hw_sensor_get_ext_name(np, binfo);
	if (binfo->ext_num == 0) {
		binfo->ext_type = NO_EXT_INFO;
		return;
	}

	if (binfo->ext_type == EXT_INFO_ADC) {
		// when use adc, need use adc channel count as ext_num
		binfo->ext_num = hw_sensor_get_ext_adc_channel(np, binfo);
		if (binfo->ext_num == 0)
			binfo->ext_type = NO_EXT_INFO;
	}

	if (binfo->ext_type == EXT_INFO_GPIO)
		binfo->adc_gpio_num = hw_sensor_get_ext_adc_gpio(np, binfo);
}

int hw_sensor_get_dt_data(struct platform_device *pdev, sensor_t *sensor)
{
	struct device_node *np = pdev->dev.of_node;
	hwsensor_board_info_t *binfo = NULL;
	int rc = 0;

	cam_debug("enter %s", __func__);
	binfo = devm_kzalloc(&pdev->dev, sizeof(hwsensor_board_info_t),
			     GFP_KERNEL);
	if (!binfo) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	binfo->dev = &(pdev->dev);

	rc = of_property_read_string(np, "huawei,sensor_name", &binfo->name);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}
	cam_debug("%s huawei,sensor_name %s, rc %d", __func__, binfo->name, rc);

	dt_read_u32(np, "vendor,is_fpga", &is_fpga);

	rc = dt_read_u32_essential(np, "huawei,sensor_index",
				   &binfo->sensor_index);
	if (rc < 0)
		return rc;

	rc = dt_read_u32_essential(np, "vendor,pd_valid",
				   &binfo->power_conf.pd_valid);
	if (rc < 0)
		return rc;

	rc = dt_read_u32_essential(np, "vendor,reset_valid",
				   &binfo->power_conf.reset_valid);
	if (rc < 0)
		return rc;

	rc = dt_read_u32_essential(np, "vendor,vcmpd_valid",
				   &binfo->power_conf.vcmpd_valid);
	if (rc < 0)
		return rc;

	dt_read_u32(np, "vendor,retry_matchid",
		    &binfo->matchid_fail_retry_flag);

	binfo->csi_id[0] = binfo->sensor_index;
	binfo->csi_id[1] = -1;
	(void)of_property_read_u32_index(np, "vendor,csi_index", 0,
					 &binfo->csi_id[0]);
	(void)of_property_read_u32_index(np, "vendor,csi_index", 1,
					 &binfo->csi_id[1]);
	cam_info("sensor:%s csi_id[0-1]=%d:%d", binfo->name, binfo->csi_id[0],
		 binfo->csi_id[1]);

	binfo->i2c_id[0] = binfo->sensor_index;
	binfo->i2c_id[1] = -1;
	(void)of_property_read_u32_index(np, "vendor,i2c_index", 0,
					 &binfo->i2c_id[0]);
	(void)of_property_read_u32_index(np, "vendor,i2c_index", 1,
					 &binfo->i2c_id[1]);
	cam_info("sensor:%s i2c_id[0-1]=%d:%d", binfo->name, binfo->i2c_id[0],
		 binfo->i2c_id[1]);

	binfo->bus_type = "";
	rc = of_property_read_string(np, "vendor,bus_type", &binfo->bus_type);
	if (rc < 0)
		cam_info("%s read bus_type failed. %d\n", __func__, __LINE__);

	binfo->tof_igs_num = TOF_IGS_INVALID;
	dt_read_u32(np, "tof-swing-num", &binfo->tof_igs_num);
	binfo->tof_gpio_num = TOF_IGS_INVALID;
	dt_read_u32(np, "tof-gpio-num", &binfo->tof_gpio_num);

	dt_read_u32(np, "vendor,ao_i2c_index", &binfo->ao_i2c_id);
	dt_read_u32(np, "vendor,igs_cam_clk", &binfo->igs_cam_clk);
	dt_read_u32(np, "vendor,i2c_pad_type", &binfo->i2c_pad_type);
	dt_read_u32(np, "vendor,phyinfo_valid", &binfo->phyinfo_valid);
	dt_read_u32(np, "vendor,is_sts", &binfo->is_sts);
	if (binfo->phyinfo_valid == 1 || binfo->phyinfo_valid == 2)
		(void)hw_sensor_get_phyinfo_data(np, binfo,
						 binfo->phyinfo_valid);

	if (!hw_is_fpga_board()) {
		binfo->topology_type = -1; // structure light, not used
		dt_read_u32(np, "module_type", &binfo->module_type);
		dt_read_u32(np, "reset_type", &binfo->reset_type);
		dt_read_u32(np, "need_rpc", &binfo->need_rpc);
		hw_sensor_get_ext_dt_data(np, binfo);
	}

	sensor->board_info = binfo;
	return 0;
}

int hw_sensor_get_dt_power_setting_data(struct platform_device *pdev,
					sensor_t *sensor)
{
	if (!pdev || !pdev->dev.of_node || !sensor) {
		cam_err("%s dev_node is NULL", __func__);
		return -EINVAL;
	}

	if (is_fpga) {
		cam_info("is fpga, skip get power setting");
		return 0;
	}

	sensor->power_group = parse_power_setting(&pdev->dev);
	if (!sensor->power_group) {
		cam_err("%pOF fail to parse power setting", pdev->dev.of_node);
		return -EINVAL;
	}
	return 0;
}

void hw_camdrv_msleep(unsigned int ms)
{
	struct timespec64 now;
	unsigned long jiffies; /*lint !e578 */

	if (ms > 5) { /* only over 5ms use schedule */
		now.tv_sec = ms / 1000;
		now.tv_nsec = (ms % CAM_MICROSECOND_PER_MILISECOND) *
			      CAM_NANOSECOND_PER_MILISECOND;
		jiffies = timespec64_to_jiffies(&now);
		schedule_timeout_interruptible(jiffies);
	} else {
		mdelay(ms);
	}
}
EXPORT_SYMBOL(hw_camdrv_msleep);

int hw_is_fpga_board(void)
{
	cam_debug("%s is_fpga=%d", __func__, is_fpga);
	return is_fpga;
}
EXPORT_SYMBOL(hw_is_fpga_board);
