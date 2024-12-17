/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: Implement of hisensor.
 * Create: 2017-04-10
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/rpmsg.h>
#include <platform_include/cee/linux/adc.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "../sensor_power.h"
#include <linux/io.h>
#include <stddef.h>
#include <platform_include/see/efuse_driver.h>
#ifndef CONFIG_KERNEL_CAMERA_ISPV360
#include "smart/drivers/channels/io_die/sts.h"
#endif

#define to_sensor(i) container_of(i, sensor_t, intf)
#define GPIO_LOW_STATE (0)
#define GPIO_HIGH_STATE (1)
#define CTL_RESET_HOLD (0)
#define CTL_RESET_RELEASE (1)

enum gpio_status {
	GPIO_SUSPEND = 0,
	GPIO_GND = 1,
	GPIO_HIGH = 2,
};

static bool g_voted;

#define CAMERA_RPC_WORKQUEUE "camera_rpc_workqueue"

int __weak rpc_status_change_for_camera(unsigned int status)
{
	pr_err("rpc_status_change_for_camera is invalid;\n");
	return -EINVAL;
}
//lint -save -e846 -e514 -e866 -e429 -e605 -e30 -e84 -e785 -e64 -e826
//lint -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737

static int hisensor_config(hwsensor_intf_t *si, void *argp);

static inline bool has_valid_board_info(hwsensor_intf_t *si)
{
	if (!si)
		return false;
	return to_sensor(si) && to_sensor(si)->board_info;
}

char const *hisensor_get_name(hwsensor_intf_t *si)
{
	if (!has_valid_board_info(si)) {
		cam_err("%s. si or board_info is NULL", __func__);
		return NULL;
	}
	return to_sensor(si)->board_info->name;
}

/* static method, caller should make sure @si not null */
static int hisensor_do_hw_buckle_check(hwsensor_intf_t *si, void *data, int tag)
{
	int status = -1;
	sensor_t *sensor = to_sensor(si);
	struct sensor_cfg_data *cdata = data;
	const char *label = "unknown";

	if (tag == FSIN) {
		status = sensor_fsin_check(sensor->power_group);
		label = "fsin";
	} else if (tag == BTB_CHECK) {
		status = sensor_btb_check(sensor->power_group);
		label = "btb_check";
	}

	cdata->data = status;
	if (status == GPIO_LOW_STATE)
		cam_info("%s.gpio:%d status is %d, camera %d is ok!", __func__,
			 label, status, tag);
	else if (status == GPIO_HIGH_STATE)
		cam_err("%s.gpio:%s status is %d, camera %d is broken!",
			__func__, label, status, tag);
	else
		cam_err("%s.gpio:%s status is %d, camera gpio_get_value is wrong!",
			__func__, label, status);

	return status;
}

static int hisensor_power_control(hwsensor_intf_t *si, bool on,
				  const char *func)
{
	int ret = 0;
	sensor_t *sensor = to_sensor(si);
	hwsensor_board_info_t *b_info;

	if (!has_valid_board_info(si)) {
		cam_err("%s. si or board_info invalid", func);
		return -EINVAL;
	}

	b_info = sensor->board_info;
	cam_info("enter %s. index = %d name = %s", func, b_info->sensor_index,
		 b_info->name);

	if (hw_is_fpga_board())
		ret = on ? do_sensor_power_on(b_info->sensor_index,
					      b_info->name) :
			   do_sensor_power_off(b_info->sensor_index,
					       b_info->name);
	else
		ret = on ? hw_sensor_power_up(sensor) :
			   hw_sensor_power_down(sensor);

	if (ret == 0)
		cam_info("%s. power sensor success", func);
	else
		cam_err("%s. power sensor fail", func);
	return ret;
}

#ifndef CONFIG_KERNEL_CAMERA_ISPV360
static sts_sys_status_enum sts_status_get_for_cam(void)
{
	return sts_get_status();
}

static int sts_normal_request_for_cam(void)
{
	int ret = 0;
	sts_ap_user_enum user = STS_USER_CAMERA;
	if (!g_voted) {
		ret = sts_normal_request(user);
		if (ret == 0)
			g_voted = true;
	}
	return ret;
}

static int sts_normal_release_for_cam(void)
{
	int ret = 0;
	sts_ap_user_enum user = STS_USER_CAMERA;
	if (g_voted) {
		ret = sts_normal_release(user);
		g_voted = false;
	}
	return ret;
}

int sts_vote_for_cam(hwsensor_intf_t *si, bool on)
{
	int ret = 0;
	sensor_t *sensor = to_sensor(si);

	sts_sys_status_enum status = STS_ST_BEGIN;
	if (sensor->board_info->is_sts == 1) {
		status = sts_status_get_for_cam();
		cam_info("sts status:%d", status);
		if (status == STS_ST_ERROR)
			return -1;
		if (status == STS_ST_INITED)
			ret = on ? sts_normal_request_for_cam() :
				   sts_normal_release_for_cam();
	}

	return ret;
}
#else
int sts_vote_for_cam(hwsensor_intf_t *si, bool on)
{
	(void)si;
	(void)on;
	cam_info("should not be here!");
	return 0;
}
#endif

int hisensor_power_up(hwsensor_intf_t *si)
{
	int ret = hisensor_power_control(si, true, __func__);
	if (ret)
		return ret;

	return sts_vote_for_cam(si, true);
}

int hisensor_power_down(hwsensor_intf_t *si)
{
	sts_vote_for_cam(si, false);
	return hisensor_power_control(si, false, __func__);
}

int hisensor_csi_enable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

int hisensor_csi_disable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

int hisensor_get_gpio_status(sensor_t *sensor, unsigned int adc_gpio_index)
{
	struct pinctrl *p = NULL;
	struct pinctrl_state *pinctrl_def = NULL;
	struct pinctrl_state *pinctrl_idle = NULL;
	int ret = -1;
	int gpio_val = 0;
	int adc_gpio_id;

	if (!sensor)
		return -1;

	p = devm_pinctrl_get(sensor->dev);
	if (!p) {
		cam_err("could not get pinctrl");
		return -1;
	}

	adc_gpio_id = sensor->board_info->adc_gpio[adc_gpio_index];
	if (gpio_request(adc_gpio_id, "adc") < 0) {
		cam_err("%s failed to request gpio[%d]", __func__, adc_gpio_id);
		goto out_put_pinctrl;
	}

	cam_info("%s gpio[%d]", __func__, adc_gpio_id);
	pinctrl_def = pinctrl_lookup_state(p, "default");
	if (!pinctrl_def) {
		cam_err("could not get defstate");
		goto out_free_gpio;
	}

	pinctrl_idle = pinctrl_lookup_state(p, "idle");
	if (!pinctrl_def) {
		cam_err("could not get idle defstate");
		goto out_free_gpio;
	}

	/* select default state. according to iomux.xml config */
	if (pinctrl_select_state(p, pinctrl_def) != 0) {
		cam_err("could not set pins to default state");
		goto out_free_gpio;
	}

	udelay(10);
	if (gpio_direction_input(adc_gpio_id) < 0) {
		cam_err("%s failed to config gpio %d input", __func__,
			adc_gpio_id);
		goto out_free_gpio;
	}

	gpio_val = gpio_get_value(adc_gpio_id);
	if (gpio_val == 0) {
		cam_info("gpio val is low");
		ret = GPIO_GND;
		goto out_free_gpio;
	}
	/* select idle state. according to iomux.xml config */
	if (pinctrl_select_state(p, pinctrl_idle) != 0) {
		cam_err("could not set pins to idle state");
		goto out_free_gpio;
	}
	udelay(10);
	if (gpio_direction_input(adc_gpio_id) < 0) {
		cam_err("%s failed to config gpio%d input", __func__,
			adc_gpio_id);
		goto out_free_gpio;
	}
	gpio_val = gpio_get_value(adc_gpio_id);
	if (gpio_val == 0) {
		cam_info("gpio val is suspend");
		ret = GPIO_SUSPEND;
	} else {
		cam_info("gpio val is high");
		ret = GPIO_HIGH;
	}

out_free_gpio:
	gpio_free(adc_gpio_id);
out_put_pinctrl:
	devm_pinctrl_put(p);
	return ret;
}

static int copy_ext_name(sensor_t *sensor, struct sensor_cfg_data *cdata,
			 int idx)
{
	int ret;

	if (idx < 0 || idx >= EXT_NAME_NUM) {
		cam_err("%s: trying copy ext_name[%d], exceeds %d", __func__,
			idx, EXT_NAME_NUM);
		return -EINVAL;
	}

	ret = strcpy_s(cdata->info.extend_name, DEVICE_NAME_SIZE,
		       sensor->board_info->ext_name[idx]);
	if (ret != 0)
		cam_err("%s:strcpy_s fail, error = %d", __func__, ret);
	else
		cam_info("%s: ext_type: %d, vcell ext_name: %s", __func__,
			 sensor->board_info->ext_type,
			 sensor->board_info->ext_name[idx]);
	return ret;
}

static int get_ext_name(sensor_t *sensor, struct sensor_cfg_data *cdata)
{
	int i = 0;
	int volt = 0;
	int max = 0;
	int min = 0;
	int gpio_status;

	if (!sensor || !cdata) {
		cam_err("%s. si or data is NULL", __func__);
		return -EINVAL;
	}

	if (sensor->board_info->ext_type == EXT_INFO_NO_ADC)
		return copy_ext_name(sensor, cdata, 0);

	if (sensor->board_info->ext_type == EXT_INFO_ADC) {
		volt = lpm_adc_get_value(sensor->board_info->adc_channel);
		if (volt < 0) { // negative means get adc fail
			cam_err("get volt fail\n");
			return -EINVAL;
		}
		cam_info("get adc value = %d\n", volt);
		// each ext_name has two adc threshold
		for (i = 0; i < sensor->board_info->ext_num / 2; ++i) {
			max = sensor->board_info->adc_threshold[i * 2];
			min = sensor->board_info->adc_threshold[i * 2 + 1];
			if ((volt < max) && (volt > min))
				return copy_ext_name(sensor, cdata, i);
		}
	}

	/*
	 * if there is only 1 gpio for detect, determine the ext_name
	 * according to the status of GPIO; if there are 2 gpios,
	 * determine the ext_name according to index of the GND gpio.
	 */
	if (sensor->board_info->ext_type == EXT_INFO_GPIO) {
		if (sensor->board_info->adc_gpio_num == 1) {
			/* select the only one adc_gpio, so the index is 0 */
			gpio_status = hisensor_get_gpio_status(sensor, 0);
			if (gpio_status < 0) {
				cam_err("get gpio status fail\n");
				return -1;
			}
			return copy_ext_name(sensor, cdata, gpio_status);
		}
		if (sensor->board_info->adc_gpio_num == 2) {
			/* find the index of GND gpio */
			for (i = 0; i < sensor->board_info->adc_gpio_num; i++) {
				gpio_status =
					hisensor_get_gpio_status(sensor, i);
				if (gpio_status < 0) {
					cam_err("get gpio status fail\n");
					return -1;
				}
				if (gpio_status == GPIO_GND)
					return copy_ext_name(sensor, cdata, i);
			}
			if (i == sensor->board_info->adc_gpio_num) {
				cam_err("read vendor info fail\n");
				return -1;
			}
		} else {
			cam_err("adc_gpio_num is wrong\n");
			return -1;
		}
	}

	return 0;
}

static int hisensor_match_id(hwsensor_intf_t *si, void *data)
{
	sensor_t *sensor = to_sensor(si);
	struct sensor_cfg_data *cdata = data;

	if (!has_valid_board_info(si) || !data) {
		cam_err("%s. si or data is NULL", __func__);
		return -EINVAL;
	}

	cam_info("%s enter", __func__);
	cdata->data = sensor->board_info->sensor_index;
	cdata->info.extend_name[0] = 0;

	if (sensor->board_info->ext_type != 0)
		get_ext_name(sensor, cdata);
	return 0;
}

static hwsensor_vtbl_t hisensor_vtbl = {
	.get_name = hisensor_get_name,
	.config = hisensor_config,
	.power_up = hisensor_power_up,
	.power_down = hisensor_power_down,
	.match_id = hisensor_match_id,
	.csi_enable = hisensor_csi_enable,
	.csi_disable = hisensor_csi_disable,
};

static void hisensor_deliver_camera_status(struct work_struct *work)
{
	rpc_info_t *rpc_info = container_of(work, rpc_info_t, rpc_work);

	if (!work) {
		cam_err("%s work is null", __func__);
		return;
	}

	if (rpc_status_change_for_camera(rpc_info->camera_status))
		cam_err("%s set_rpc %d fail", __func__,
			rpc_info->camera_status);
	else
		cam_info("%s set_rpc %d success", __func__,
			 rpc_info->camera_status);
}

/* camera radio power ctl for radio frequency interference */
static int hisensor_do_rpc_config(hwsensor_board_info_t *b_info, int rpc_state)
{
	if (rpc_state != SENSOR_RPC_ON && rpc_state != SENSOR_RPC_OFF) {
		cam_err("%s invalid rpc_state %d", __func__, rpc_state);
		return -EINVAL;
	}

	cam_info("%s set_rpc = %d for work queue", __func__, rpc_state);
	if (!(b_info->rpc_info.rpc_work_queue)) {
		cam_err("%s invalid params rpc_work_queue", __func__);
		return -EINVAL;
	}
	b_info->rpc_info.camera_status = rpc_state; /* set rpc */
	queue_work(b_info->rpc_info.rpc_work_queue,
		   &(b_info->rpc_info.rpc_work));

	return 0;
}

static int hisensor_do_hw_reset(hwsensor_intf_t *si, int ctl, int id)
{
	sensor_t *sensor = to_sensor(si);
	hwsensor_board_info_t *b_info = NULL;
	int ret = 0;

	if (!has_valid_board_info(si)) {
		cam_err("%s. si or board_info is null", __func__);
		return -EINVAL;
	}

	b_info = sensor->board_info;
	cam_info("reset_type = %d set reset state %s id %d", b_info->reset_type,
		 (ctl == CTL_RESET_RELEASE ? "release" : "hold"), id);

	if (b_info->need_rpc == RPC_SUPPORT) {
		cam_info("%s: need rpc = %d", __func__, b_info->need_rpc);
		if (ctl == CTL_RESET_RELEASE)
			(void)hisensor_do_rpc_config(b_info, SENSOR_RPC_ON);
		else
			(void)hisensor_do_rpc_config(b_info, SENSOR_RPC_OFF);
	}
	ret = sensor_release_hold(sensor->power_group, ctl == CTL_RESET_RELEASE);
	if (ret)
		cam_err("%s set reset pin failed", __func__);
	return ret;
}

#ifdef CONFIG_SCSI_UFS_SCORPIO
extern void ufshcd_auto_hibern8_open(void);
extern void ufshcd_auto_hibern8_close(void);
static int ufs_vote_count;
#endif
static void hisensor_do_ufs_control(hwsensor_intf_t *si, bool auto_control)
{
#ifdef CONFIG_SCSI_UFS_SCORPIO
	if (auto_control) {
		if (ufs_vote_count == 1) {
			ufs_vote_count = 0;
			ufshcd_auto_hibern8_open();
		}
	} else {
		if (ufs_vote_count == 0) {
			ufs_vote_count = 1;
			ufshcd_auto_hibern8_close();
		}
	}
	cam_err("%s set ufs control %d, vote_cnt %d", __func__, auto_control, ufs_vote_count);
#endif
	return;
}

#define READ_BITS 20
#define MIN_BUF_SIZE efuse_div_round_up(READ_BITS, EFUSE_BITS_PER_GROUP)
static int hisensor_do_get_efuseid(hwsensor_intf_t *si, void *data)
{
	uint32_t ret = 0;
	struct sensor_cfg_data *cdata = data;
	uint32_t buf[MIN_BUF_SIZE] = { 0 };
	struct efuse_desc desc = { 0 };

	if (!data) {
		cam_err("%s. data is NULL", __func__);
		return -EINVAL;
	}

	desc.buf = buf;
	desc.buf_size = MIN_BUF_SIZE;
	desc.start_bit = 0;
	desc.bit_cnt = READ_BITS;
	desc.item_vid = EFUSE_KERNEL_RXCPHY_MASK;
	ret = efuse_read_value_t(&desc);
	if (ret != EFUSE_OK) {
		cam_err("%s failed", __func__);
		return ret;
	}

	cdata->data = desc.buf[0];
	cam_info("%s efuseid:%d", __func__, cdata->data);
	return ret;
}

static int hisensor_config(hwsensor_intf_t *si, void *argp)
{
	int ret = 0;
	struct sensor_cfg_data *data = argp;
	sensor_t *sensor = to_sensor(si);
	hwsensor_board_info_t *b_info = NULL;

	if (!si || !data || !si->vtbl) {
		cam_err("%s si or argp is null\n", __func__);
		return -EINVAL;
	}

	if (!has_valid_board_info(si)) {
		cam_err("%s si or board_info is null\n", __func__);
		return -EINVAL;
	}
	b_info = sensor->board_info;

	data = (struct sensor_cfg_data *)argp;
	cam_info("hisensor cfgtype = %d", data->cfgtype);

	switch (data->cfgtype) {
	case SEN_CONFIG_POWER_ON:
		if (!si->vtbl->power_up) {
			cam_err("%s. si->vtbl->power_up is null", __func__);
			ret = -EINVAL;
		} else {
			ret = si->vtbl->power_up(si);
			if (ret != 0)
				cam_err("%s. power up fail", __func__);
		}
		break;

	case SEN_CONFIG_POWER_OFF:
		if (!si->vtbl->power_down) {
			cam_err("%s. si->vtbl->power_down is null", __func__);
			ret = -EINVAL;
		} else {
			ret = si->vtbl->power_down(si);
			if (ret != 0)
				cam_err("%s. power_down fail", __func__);
		}

		if (b_info->need_rpc == RPC_SUPPORT_DYNAMIC) {
			ret = hisensor_do_rpc_config(b_info, SENSOR_RPC_OFF);
			cam_info("%s: cfgtype(POWER_OFF), rpc_state%d",
				 __func__, data->data);
		}
		break;
	case SEN_CONFIG_EXT_VCM_POWER_ON:
		ret = sensor_ext_vcm_power_on_off(sensor->power_group, true);
		break;
	case SEN_CONFIG_EXT_VCM_POWER_OFF:
		ret = sensor_ext_vcm_power_on_off(sensor->power_group, false);
		break;
	case SEN_CONFIG_WRITE_REG:
	case SEN_CONFIG_READ_REG:
	case SEN_CONFIG_WRITE_REG_SETTINGS:
	case SEN_CONFIG_READ_REG_SETTINGS:
	case SEN_CONFIG_ENABLE_CSI:
	case SEN_CONFIG_DISABLE_CSI:
		break;
	case SEN_CONFIG_MATCH_ID:
		if (!si->vtbl->match_id) {
			cam_err("%s. si->vtbl->match_id is null", __func__);
			ret = -EINVAL;
		} else {
			ret = si->vtbl->match_id(si, argp);
		}
		break;
	case SEN_CONFIG_RESET_HOLD:
		ret = hisensor_do_hw_reset(si, CTL_RESET_HOLD, data->mode);
		break;
	case SEN_CONFIG_RESET_RELEASE:
		ret = hisensor_do_hw_reset(si, CTL_RESET_RELEASE, data->mode);
		break;
	case SEN_CONFIG_UFS_AUTO_CONTROL_ON:
		hisensor_do_ufs_control(si, true);
		break;
	case SEN_CONFIG_UFS_AUTO_CONTROL_OFF:
		hisensor_do_ufs_control(si, false);
		break;
	case SEN_CONFIG_MIPI_SWITCH:
		ret = sensor_mipi_switch(sensor->power_group);
		break;
	case SEN_CONFIG_FPC_CHECK:
		ret = hisensor_do_hw_buckle_check(si, argp, FSIN);
		break;
	case SEN_CONFIG_BTB_CHECK:
		ret = hisensor_do_hw_buckle_check(si, argp, BTB_CHECK);
		break;
	case SEN_CONFIG_RPC_STATE:
		if (b_info->need_rpc == RPC_SUPPORT_DYNAMIC) {
			ret = hisensor_do_rpc_config(b_info, data->data);
			cam_info("%s cfgtype(CONFIG_RPC), rpc_state%d",
				 __func__, data->data);
		}
		break;
	case SEN_CONFIG_GET_EFUSE_ID:
		ret = hisensor_do_get_efuseid(si, argp);
		break;
	default:
		cam_err("%s cfgtype %d is error", __func__, data->cfgtype);
		break;
	}

	cam_debug("%s exit %d", __func__, ret);
	return ret;
}

static void init_rpc_workqueue(hwsensor_board_info_t *b_info)
{
	if (!b_info) {
		cam_err("%s invalid params b_info", __func__);
		return;
	}
	if ((b_info->need_rpc != RPC_SUPPORT) &&
	    (b_info->need_rpc != RPC_SUPPORT_DYNAMIC))
		return;

	INIT_WORK(&(b_info->rpc_info.rpc_work), hisensor_deliver_camera_status);

	b_info->rpc_info.rpc_work_queue =
		create_singlethread_workqueue(CAMERA_RPC_WORKQUEUE);

	if (!b_info->rpc_info.rpc_work_queue) {
		cam_err("%s create rpc workqueue error", __func__);
		return;
	}

	cam_info("%s create rpc workqueue success", __func__);
}

static int32_t hisensor_platform_probe(struct platform_device *pdev)
{
	int rc = 0;
	sensor_t *s_ctrl = NULL;

	cam_info("%s enter", __func__);
	if (NULL == pdev) {
		cam_err("%s:%d pdev is null.\n", __func__, __LINE__);
		return -EINVAL;
	}

	s_ctrl = devm_kzalloc(&pdev->dev, sizeof(sensor_t), GFP_KERNEL);
	if (!s_ctrl) {
		cam_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		return -ENOMEM;
	}

	rc = hw_sensor_get_dt_data(pdev, s_ctrl);
	if (rc < 0) {
		cam_err("%s:%d no dt data rc %d", __func__, __LINE__, rc);
		goto hisensor_probe_fail;
	}

	rc = hw_sensor_get_dt_power_setting_data(pdev, s_ctrl);
	if (rc < 0) {
		cam_err("%s:%d no dt power setting data rc %d", __func__,
			__LINE__, rc);
		goto hisensor_probe_fail;
	}

	s_ctrl->intf.vtbl = &hisensor_vtbl;
	s_ctrl->dev = &pdev->dev;
	rc = hwsensor_register(pdev, &(s_ctrl->intf));
	if (rc != 0) {
		cam_err("%s:%d hwsensor_register fail.\n", __func__, __LINE__);
		goto hisensor_probe_fail;
	}

	rc = rpmsg_sensor_register(pdev, (void *)s_ctrl);
	if (rc != 0) {
		cam_err("%s:%d rpmsg_sensor_register fail.\n", __func__,
			__LINE__);
		hwsensor_unregister(pdev);
		goto hisensor_probe_fail;
	}

	init_rpc_workqueue(s_ctrl->board_info);
hisensor_probe_fail:
	return rc;
}

static void destory_rpc_workqueue(hwsensor_board_info_t *b_info)
{
	if (!b_info) {
		cam_err("%s invalid params b_info", __func__);
		return;
	}
	if (b_info->need_rpc != RPC_SUPPORT &&
	    b_info->need_rpc != RPC_SUPPORT_DYNAMIC)
		return;
	if (!b_info->rpc_info.rpc_work_queue)
		return;
	destroy_workqueue(b_info->rpc_info.rpc_work_queue);
	b_info->rpc_info.rpc_work_queue = NULL;
	cam_info("%s: destroy rpc workqueue success", __func__);
}

static int32_t hisensor_platform_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	sensor_t *s_ctrl = NULL;

	if (!sd) {
		cam_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	s_ctrl = (sensor_t *)v4l2_get_subdevdata(sd);
	if (!s_ctrl) {
		cam_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	destory_rpc_workqueue(s_ctrl->board_info);
	rpmsg_sensor_unregister((void *)s_ctrl);
	hwsensor_unregister(pdev);
	return 0;
}

static const struct of_device_id hisensor_dt_match[] = {
	{
		.compatible = "huawei,sensor",
	},
	{},
};

MODULE_DEVICE_TABLE(of, hisensor_dt_match);

static struct platform_driver hisensor_platform_driver = {
	.driver = {
		.name           = "cam_sensor",
		.owner          = THIS_MODULE,
		.of_match_table = hisensor_dt_match,
	},

	.probe = hisensor_platform_probe,
	.remove = hisensor_platform_remove,
};

static int __init hisensor_init_module(void)
{
	cam_info("enter %s", __func__);
	return platform_driver_register(&hisensor_platform_driver);
}

static void __exit hisensor_exit_module(void)
{
	platform_driver_unregister(&hisensor_platform_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(hisensor_init_module);
module_exit(hisensor_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("hisensor");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
