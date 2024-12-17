// SPDX-License-Identifier: GPL-2.0
/* star_module_adapter.c
 *
 * star_module_adapter driver
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

#include "cam_star_module_adapter.h"
#include "cam_log.h"
#include <huawei_platform/log/hw_log.h>
#include <platform_include/camera/native/camera.h>
#include <dsm/dsm_pub.h>
#include <linux/videodev2.h>
#include <media/media-entity.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <media/v4l2-image-sizes.h>
#include <media/v4l2-mediabus.h>
#include <securec.h>
#include <cam_intf.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <chipset_common/hwmanufac/dev_detect/dev_detect.h>
#endif

#define HWLOG_TAG cam_star_module_adapter
HWLOG_REGIST();

extern int stm32g031_probe(struct i2c_client *client, const struct i2c_device_id *id, struct cam_star_module_device_info *di);
extern int stm32g031_remove(struct i2c_client *client);
extern int hc32l110_probe(struct i2c_client *client, const struct i2c_device_id *id, struct cam_star_module_device_info *di);
extern int hc32l110_remove(struct i2c_client *client);

static struct cam_star_module_intf_t star_ic_devices[] = {
	{ "stm32g031", &stm32g031_probe, &stm32g031_remove },
	{ "hc32l110", &hc32l110_probe, &hc32l110_remove },
};

static int cam_parse_gpio(struct device_node *np,
	struct cam_star_module_device_info *di)
{
	int rc = 0;
	char *gpio_tag = NULL;
	const char *gpio_ctrl_types[GPIO_MAX] = {"MCU_INT", "MCU_BOOT", "MCU_RESET", "MOTOR_EN", "UART_TX", "UART_RX"};
	int gpio[GPIO_MAX] = {-1, -1, -1, -1, -1, -1, -1};
	int index = 0;
	int i = 0;
	int gpio_num = 0;

	if (!np || !di) {
		hwlog_err("np or di is null\n");
		return -EINVAL;
	}

	di->power_setting.mcu_int_gpio = -1;
	di->power_setting.mcu_boot_gpio = -1;
	di->power_setting.mcu_reset_gpio = -1;
	di->power_setting.motor_en_gpio = -1;

	gpio_num = of_gpio_count(np);
	if (gpio_num <= 0) {
		hwlog_err("can't find gpio\n");
		return -EINVAL;
	}
	for (index = 0; index < gpio_num; index++) {
		rc = of_property_read_string_index(np, "huawei,gpio-ctrl-types", index, (const char **)&gpio_tag);
		if (rc < 0) {
			hwlog_err("can't find gpio-ctrl-types\n");
			return -EINVAL;
		}
		for (i = 0; i < GPIO_MAX; i++) {
			if (!strcmp(gpio_ctrl_types[i], gpio_tag)) {
				gpio[i] = of_get_gpio(np, index);
				break;
			}
		}
		hwlog_info("gpio ctrl types: %s gpio = %d\n", gpio_tag, gpio[i]);
		if (!gpio_is_valid(gpio[i])) {
			hwlog_err("gpio %d is invalid\n", gpio[i]);
			return -EINVAL;
		}
		if (gpio_request(gpio[i], gpio_tag)) {
			hwlog_err("gpio %d request fail\n", gpio[i]);
			return -EINVAL;
		}
	}
	di->power_setting.mcu_int_gpio = gpio[MCU_INT];
	di->power_setting.mcu_boot_gpio = gpio[MCU_BOOT];
	di->power_setting.mcu_reset_gpio = gpio[MCU_RESET];
	di->power_setting.motor_en_gpio = gpio[MOTOR_EN];

	return 0;
}

static int cam_star_module_parse_dts(struct device_node *np,
	struct cam_star_module_device_info *di)
{
	int rc = 0;

	if (of_property_read_string(np, "ic_name", &di->ic_name)) {
		hwlog_err("has't ic_name!\n");
		return -EINVAL;
	}

	rc = cam_parse_gpio(np, di);
	if (rc) {
		hwlog_err("pase gpio error.\n");
		return -EINVAL;
	}

	return rc;
}

static int ic_probe(struct i2c_client *client,
	const struct i2c_device_id *id, struct cam_star_module_device_info *di)
{
	int rc = 0;
	int i = 0;

	if (!client || !id || !di) {
		hwlog_err("param null\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(star_ic_devices); i++) {
		if (!strcmp(star_ic_devices[i].ic_name, di->ic_name)) {
			rc = star_ic_devices[i].ic_init(client, id, di);
			if (rc)
				hwlog_err("ic %s probe failed, now free irq.\n", star_ic_devices[i].ic_name);
			break;
		}
	}

	if (i == ARRAY_SIZE(star_ic_devices)) {
		hwlog_err("No star_ic_devices match power tree device!\n");
		return -EINVAL;
	}

	return rc;
}

static long star_ioctl_select(struct cam_star_module_device_info *di,
	unsigned int cmd, void *arg)
{
	long rc = 0;
	u16 code = 0;
	u16 speed = 0;

	if (!di || !arg) {
		hwlog_err("param null\n");
		return -EINVAL;
	}

	if (!di->api_func_tbl) {
		hwlog_err("invalid func table!\n");
		return -EINVAL;
	}

	mutex_lock(&di->lock);
	switch (cmd) {
	case HWSENSORMOTOR_IOCTL_GET_INFO:
		hwlog_info("Enter HWSENSORMOTOR_IOCTL_GET_INFO\n");
		struct sensormotor_info *p = NULL;
		if (di->api_func_tbl->get_info) {
			struct sensormotor_info ic_info;
			rc = di->api_func_tbl->get_info(di, &ic_info);
			p = (struct sensormotor_info *)arg;
			if (memcpy_s((struct sensormotor_info *)arg, sizeof(struct sensormotor_info), &ic_info, sizeof(struct sensormotor_info)) != EOK) {
				hwlog_err("memcpy_s failed.\n");
				mutex_unlock(&di->lock);
				return -EINVAL;
			}
			hwlog_info("ic_name: %s\n", (struct sensormotor_info *)p->name, arg);
		}
		break;
	/* repaired tag before crc_check */
	case HWSENSORMOTOR_IOCTL_MAINTENANCE_TAG:
		hwlog_info("Enter HWSENSORMOTOR_IOCTL_MAINTENANCE_TAG\n");
			di->maintenance_state = REPAIRED;
		break;
	case HWSENSORMOTOR_IOCTL_DOWNLOAD_FW:
		hwlog_info("enter HWSENSORMOTOR_IOCTL_DOWNLOAD_FW\n");
		if (di->api_func_tbl->download_firmware &&
			di->api_func_tbl->reload_calib_data_from_eeprom &&
			di->api_func_tbl->flash_crc_check) {
			rc = di->api_func_tbl->download_firmware(di);
			rc += di->api_func_tbl->reload_calib_data_from_eeprom(di);
			rc += di->api_func_tbl->flash_crc_check(di);
		}
		break;
	case HWSENSORMOTOR_IOCTL_POWER_ON:
		hwlog_info("Enter HWSENSORMOTOR_IOCTL_POWER_ON\n");
		if (di->api_func_tbl->power_on)
			rc = di->api_func_tbl->power_on(di);
		break;
	case HWSENSORMOTOR_IOCTL_POWER_OFF:
		hwlog_info("Enter HWSENSORMOTOR_IOCTL_POWER_OFF\n");
		if (di->api_func_tbl->power_off)
			rc = di->api_func_tbl->power_off(di, *(enum power_off_state_t *)arg);
		break;
	case HWSENSORMOTOR_IOCTL_MAINTENANCE_DELTA:
		hwlog_info("enter HWSENSORMOTOR_IOCTL_MAINTENANCE_DELTA\n");
		if (di->api_func_tbl->set_maintenance_delta) {
			hwlog_info("set_maintenance_delata: %d\n", *(u16 *)arg);
			rc += di->api_func_tbl->set_maintenance_delta(di, *(u16 *)arg);
		}
		break;
	case HWSENSORMOTOR_IOCTL_MOVE:
		hwlog_info("Enter HWSENSORMOTOR_IOCTL_MOVE\n");
		struct sensormotor_move_info * p_move_info = NULL;
		if (di->normal_calib_state == NORMAL) {
			hwlog_info("enter nomal mode move.\n");
			if (di->api_func_tbl->set_code &&
				di->api_func_tbl->set_speed &&
				di->api_func_tbl->start) {
				p_move_info = (struct sensormotor_move_info*)arg;
				hwlog_info("normal mode set code: %u, speed: %u\n", p_move_info->code, p_move_info->speed);
				rc += di->api_func_tbl->set_code(di, p_move_info->code);
				rc += di->api_func_tbl->set_speed(di, p_move_info->speed);
				rc += di->api_func_tbl->start(di);
			}
		} else if (di->normal_calib_state == CALIB) {
			hwlog_info("enter calib mode move.\n");
			if (di->api_func_tbl->calib_set_step &&
				di->api_func_tbl->calib_set_speed &&
				di->api_func_tbl->calib_start) {
				p_move_info = (struct sensormotor_move_info*)arg;
				hwlog_info("calib mode set step: %d, speed: %u\n", p_move_info->step, p_move_info->speed);
				rc += di->api_func_tbl->calib_set_step(di, p_move_info->step);
				rc += di->api_func_tbl->calib_set_speed(di, p_move_info->speed);
				rc += di->api_func_tbl->calib_start(di);
			}
		} else {
			hwlog_err("invalid mode.\n");
			rc = -EINVAL;
		}
		break;
	case HWSENSORMOTOR_IOCTL_GET_REAL_HEIGHT:
		hwlog_info("Enter HWSENSORMOTOR_IOCTL_GET_REAL_HEIGHT\n");
		if (di->normal_calib_state == NORMAL) {
			hwlog_info("enter nomal mode get_real_height.\n");
			if (di->api_func_tbl->get_real_height) {
				u16 real_code = 0;
				rc = di->api_func_tbl->get_real_height(di, &real_code);
				if (memcpy_s((u16 *)arg, sizeof(u16), &real_code, sizeof(u16)) != EOK) {
					hwlog_err("get_real_height memcpy_s failed\n");
					mutex_unlock(&di->lock);
					return -EINVAL;
				}
				hwlog_info("get_real_height: %u\n", real_code);
			}
		} else if (di->normal_calib_state == CALIB) {
			hwlog_info("enter calib mode get_real_height.\n");
			if (di->api_func_tbl->calib_get_real_height) {
				u16 real_code = 0;
				rc = di->api_func_tbl->calib_get_real_height(di, &real_code);
				if (memcpy_s((u16 *)arg, sizeof(u16), &real_code, sizeof(u16)) != EOK) {
					hwlog_err("get_real_height memcpy_s failed\n");
					mutex_unlock(&di->lock);
					return -EINVAL;
				}
				hwlog_info("calib mode get_real_height: %u\n", real_code);
			}
		} else {
			hwlog_err("invalid mode.\n");
			rc = -EINVAL;
		}
		break;
	case HWSENSORMOTOR_IOCTL_STOP:
		hwlog_info("Enter HWSENSORMOTOR_IOCTL_STOP\n");
		if (di->api_func_tbl->stop)
			rc = di->api_func_tbl->stop(di);
		break;
	case HWSENSORMOTOR_IOCTL_SET_CALIB_MODE:
		hwlog_info("Enter HWSENSORMOTOR_IOCTL_SET_CALIB_MODE\n");
		if (di->api_func_tbl->set_calib_mode)
			rc = di->api_func_tbl->set_calib_mode(di, *(enum calib_mode_t *)arg);
		hwlog_info("set_calib_mode: %d\n", *(enum calib_mode_t *)arg);
		break;
	/* specially for Maintenance outlets */
	case HWSENSORMOTOR_IOCTL_REPAIRED_TAG:
		hwlog_info("enter HWSENSORMOTOR_IOCTL_REPAIRED_TAG\n");
		if (di->api_func_tbl->set_repaired_tag)
			rc += di->api_func_tbl->set_repaired_tag(di);
		break;
	default:
		hwlog_err("invalid IOCTL CMD(0x%x)\n", cmd);
		rc = -EINVAL;
		break;
	}
	mutex_unlock(&di->lock);
	return rc;
}

static long star_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	struct cam_star_module_device_info *di = NULL;
	if (!arg || !sd) {
		hwlog_err("%s: arg or sd is NULL\n", __func__);
		return -EINVAL;
	}

	di = container_of(sd, struct cam_star_module_device_info, subdev);
	if (!di) {
		hwlog_err("%s: isp is NULL\n", __func__);
		return -EINVAL;
	}

	hwlog_info("cmd is (0x%x), arg(%pK)\n", cmd, arg);

	if (!di->api_func_tbl)
		return -EINVAL;

	return star_ioctl_select(di, cmd, arg);
}

static int star_subdev_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	int rc = 0;
	struct cam_star_module_device_info *di = NULL;
	(void)fh;

	if (!sd) {
		hwlog_err("%s: sd is NULL\n", __func__);
		return -EINVAL;
	}

	di = container_of(sd, struct cam_star_module_device_info, subdev);
	if (!di) {
		hwlog_err("%s: di is NULL\n", __func__);
		return -EINVAL;
	}

	if (di && di->api_func_tbl && di->api_func_tbl->open)
			rc = di->api_func_tbl->open(di);

	return rc;
}

static int star_subdev_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	int rc = 0;
	struct cam_star_module_device_info *di = NULL;
	(void)fh;

	if (!sd) {
		hwlog_err("%s: sd is NULL\n", __func__);
		return -EINVAL;
	}

	di = container_of(sd, struct cam_star_module_device_info, subdev);
	if (!di) {
		hwlog_err("%s: di is NULL\n", __func__);
		return -EINVAL;
	}

	if (di && di->api_func_tbl && di->api_func_tbl->close)
			rc = di->api_func_tbl->close(di);

	return rc;
}

static int star_subdev_subscribe_event(struct v4l2_subdev *sd,
	struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub)
{
	/* 128:size */
	return v4l2_event_subscribe(fh, sub, 128, NULL);
}

static int star_subdev_unsubscribe(struct v4l2_subdev *sd,
	struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

static const struct v4l2_subdev_core_ops star_subdev_core_ops = {
	.subscribe_event = star_subdev_subscribe_event,
	.unsubscribe_event = star_subdev_unsubscribe,
	.ioctl = star_subdev_ioctl,
};

static const struct v4l2_subdev_ops star_subdev_ops = {
	.core  = &star_subdev_core_ops,
};

static const struct v4l2_subdev_internal_ops star_subdev_internal_ops = {
	.open = star_subdev_open,
	.close = star_subdev_close,
};

static void hisp_notify_rpmsg_cb(struct star_notify_intf_t *i, struct star_event_t *star_ev)
{
	struct cam_star_module_device_info *di = NULL;
	struct v4l2_event ev;
	struct video_device *vdev = NULL;
	struct star_event_t *req = (struct star_event_t *)ev.u.data;
	errno_t err = memset_s(&ev, sizeof(struct v4l2_event), 0,
						   sizeof(struct v4l2_event));
	if (err != EOK)
		hwlog_err("func %s: memset_s fail\n", __func__);
	hwlog_info("enter %s\n", __func__);

	if (!star_ev || !i) {
		hwlog_err("func %s: star_ev or i is NULL\n", __func__);
		return;
	}
	di = container_of(i, struct cam_star_module_device_info, notify);
	if (!di) {
		hwlog_err("func %s: di is NULL\n", __func__);
		return;
	}
	vdev = di->subdev.devnode;

	ev.type = SENSORMOTOR_V4L2_EVENT_TYPE;
	ev.id = SENSORMOTOR_HIGH_PRIO_EVENT;
	req->kind = star_ev->kind;
	v4l2_event_queue(vdev, &ev);
}

static struct star_notify_vtbl star_notify = {
	.rpmsg_cb = hisp_notify_rpmsg_cb,
};

static struct dsm_dev dev_cam_star = {
	.name = "dsm_cam_star",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

static int cam_star_probe(struct i2c_client *client,
	const struct i2c_device_id *id, struct cam_star_module_device_info *di)
{
	int rc = 0;
	struct v4l2_subdev *sd = NULL;

	if (!client || !id || !di) {
		hwlog_err("param null\n");
		return -EINVAL;
	}

	sd = &di->subdev;
	if (!sd) {
		hwlog_err("%s: subdev is NULL\n", __func__);
		return -EINVAL;
	}
	if (!(&star_subdev_ops)) {
		hwlog_err("%s: const struct v4l2_subdev_ops *ops is NULL\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&di->lock);
	v4l2_i2c_subdev_init(sd, client, &star_subdev_ops);
	sd->internal_ops = &star_subdev_internal_ops;
	snprintf_s(sd->name, sizeof(sd->name),
					 sizeof(sd->name) - 1, "%s", "cam_star");

	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE |
		V4L2_SUBDEV_FL_HAS_EVENTS;

	init_subdev_media_entity(sd, CAM_SUBDEV_SENSORMOTOR);
	cam_cfgdev_register_subdev(sd, CAM_SUBDEV_SENSORMOTOR);

	hwlog_info("rc = %d register video devices %s sucessful, major = %d, minor = %d, entity.name = %s\n",
		rc, sd->name, sd->entity.info.dev.major,
		sd->entity.info.dev.minor, sd->entity.name);

	di->notify.vtbl = &star_notify;

	if (!di->dsm_client)
		di->dsm_client = dsm_register_client(&dev_cam_star);

	mutex_unlock(&di->lock);

	return rc;
}

static int cam_star_classdev_register(struct cam_star_module_device_info *di)
{
	int rc = 0;
	struct device *pdevice = NULL;
	di->dev_class = class_create(THIS_MODULE, "cam_star");
	if (!(di->dev_class))
		return PTR_ERR(di->dev_class);
	pdevice = device_create(di->dev_class, di->dev,
			0, NULL, "cam_star");
	if (!pdevice) {
		hwlog_err("%s: device_create error\n", __func__);
		class_destroy(di->dev_class);
		di->dev_class = NULL;
		rc = -EFAULT;
	}
	return rc;
}

static int cam_star_module_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct cam_star_module_device_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;
	int rc = 0;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;

	if (cam_star_module_parse_dts(np, di))
		goto fail_free_mem;

	// chip probe
	mutex_init(&di->lock);
	rc = ic_probe(client, id, di);
	if (!rc) {
		hwlog_info("ic %s probed success, now register v4l2.\n", client->name);
		rc = cam_star_probe(client, id, di);
		if (rc) {
			hwlog_err("register v4l2 sudev failed.\n");
			goto free_gpio;
		}
	} else {
		hwlog_err("no star ic probe success, now release resource.\n");
		goto free_gpio;
	}
	rc = cam_star_classdev_register(di);
	if (rc) {
		hwlog_err("creating classdev group failed\n");
		goto free_gpio;
	}
	rc = sysfs_create_groups(&di->dev->kobj, di->star_attr_group);
	if (rc) {
		hwlog_err("creating sysfs attribute group failed\n");
		goto free_classdev;
	}
	dev_set_drvdata(&client->dev, di);

	return rc;

free_classdev:
	class_destroy(di->dev_class);
free_gpio:
	gpio_free(di->power_setting.mcu_int_gpio);
	gpio_free(di->power_setting.mcu_boot_gpio);
	gpio_free(di->power_setting.mcu_reset_gpio);
	gpio_free(di->power_setting.motor_en_gpio);
	mutex_destroy(&di->lock);
fail_free_mem:
	devm_kfree(&client->dev, di);
	return -EPERM;
}

static int cam_star_module_remove(struct i2c_client *client)
{
	struct cam_star_module_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	mutex_destroy(&di->lock);
	devm_kfree(&client->dev, di);

	return 0;
}

MODULE_DEVICE_TABLE(i2c, cam_star_module);
static const struct of_device_id cam_star_module_of_match[] = {
	{
		.compatible = "cam_star_module",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id cam_cam_star_module_i2c_id[] = {
	{ "cam_star_module", 0 }, {}
};

static struct i2c_driver cam_star_module_driver = {
	.probe = cam_star_module_probe,
	.remove = cam_star_module_remove,
	.id_table = cam_cam_star_module_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "cam_star_module",
		.of_match_table = of_match_ptr(cam_star_module_of_match),
	},
};

static __init int cam_star_module_init(void)
{
	return i2c_add_driver(&cam_star_module_driver);
}

static __exit void cam_star_module_exit(void)
{
	i2c_del_driver(&cam_star_module_driver);
}

module_init(cam_star_module_init);
module_exit(cam_star_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cam_star_module driver");
MODULE_AUTHOR("huawei Technologies Co., Ltd.");
