/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/usb.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_drivers/pcie-kport-api.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/of_address.h>
#include <linux/mutex.h>
#ifdef CONFIG_DKMD_DKSM
#include <platform_include/display/dkmd/dkmd_notify.h>
#include <linux/fb.h>
#endif

#include <securec.h>
#include <linux/mutex.h>

#include "external_camera.h"

#define cam_err(fmt, args...) \
	do { \
		printk(KERN_ERR "[external_camera ERR] %s: %d " fmt, __func__, __LINE__, ## args); \
	} while (0)

#define cam_info(fmt, args...) \
	do { \
		printk(KERN_INFO "[external_camera INFO] %s: %d " fmt, __func__, __LINE__, ## args); \
	} while (0)

#define cam_warn(fmt, args...) \
	do { \
		printk(KERN_WARNING "[external_camera WARN] %s: %d " fmt, __func__, __LINE__, ## args); \
	} while (0)

#define GPIO_NUM_MAX    10
#define LDO_NUM_MAX     10
#define DEV_POWER_PIN_MAX 10
#define DELAY_THRESHOLD  2

#define MAX_DELAY_POWEROFF_WL_VAL 8000
#define MIN_DELAY_POWEROFF_WL_VAL 3000

#define EXCAM_DEV_CNT  1
#define EXCAM_DEV_NAME "excam"

typedef enum power_type {
	POWER_TYPE_OFF = 0,
	POWER_TYPE_ON = 1,
	POWER_TYPE_MAX = 2,
} power_type_t;

typedef enum external_device_type {
	EXTERNAL_DEVICE_TYPE_PCIE2USB = 0,
	EXTERNAL_DEVICE_TYPE_ISP = 1,
	EXTERNAL_DEVICE_TYPE_SENSOR = 2,
	EXTERNAL_DEVICE_TYPE_MAX = 3,
} external_device_type_t;

typedef enum power_status {
	POWER_STATUS_OFF = 0,
	POWER_STATUS_ON = 1,
	POWER_STATUS_MAX = 2,
} power_status_t;

typedef enum power_dts_info {
	POWER_DTS_SEQ_TYPE = 0,
	POWER_DTS_SEQ_VAL = 1,
	POWER_DTS_SEQ_DELAY = 2,
	POWER_DTS_SEQ_MAX = 3,
} power_dts_info_t;

struct excam_device {
	dev_t cam_devno;
	struct cdev excam_dev;
	struct class *class_cam;
	struct device *device;
}g_excam_device;

typedef struct gpio_def {
	const char *name;
	int gpio_no;
}gpio_t;

typedef struct ldo_def {
	const char* name;
	struct regulator *supply;
}ldo_t;

typedef struct dev_power_res {
	int num_of_ldo;
	int num_of_gpio;
	gpio_t gpios[GPIO_NUM_MAX];
	ldo_t ldos[LDO_NUM_MAX];
}dev_power_res_t;

typedef struct power_pin {
	bool is_gpio;
	ldo_t *ldo;
	gpio_t *gpio;
	const char* dts_name;
	int val;
	int delay;
}power_pin_t;

typedef struct dev_power_seq {
	int num;
	power_pin_t pins[DEV_POWER_PIN_MAX];
}dev_power_seq_t;

typedef struct pcie2usb_dev_data {
	int pcie_rc_idx;
	dev_power_seq_t poweron_seq;
	dev_power_seq_t poweroff_seq;
	dev_power_res_t power_res;
	power_status_t power_status;
	int perst_pullhigh_index;
}pcie2usb_data_t;

typedef struct isp_dev_data {
	dev_power_seq_t poweron_seq;
	dev_power_seq_t poweroff_seq;
	dev_power_res_t power_res;
	power_status_t power_status;
}isp_data_t;

typedef struct sensor_dev_data {
	int num_of_sensor;
	dev_power_res_t power_res;
	dev_power_seq_t poweron_seq[SENSOR_NUM_MAX];
	dev_power_seq_t poweroff_seq[SENSOR_NUM_MAX];
	power_status_t power_status[SENSOR_NUM_MAX];
	sensor_info_t *info;
}sensor_data_t;

typedef struct external_camera_data {
	struct device *dev;
	pcie2usb_data_t pcie2usb_data;
	isp_data_t isp_data;
	sensor_data_t sensor_data;
	int flash_invalid;
	struct wakeup_source *delay_poweroff_wl;
}external_camera_data_t;

static external_camera_data_t *g_camera;
static sensor_list_t g_all_sensor;
static struct mutex g_mutex_power;
static char* power_info_dts_string[POWER_TYPE_MAX][POWER_DTS_SEQ_MAX] = {
	{ "poweroff-seq-type", "poweroff-seq-val", "poweroff-seq-delay" },
	{ "poweron-seq-type", "poweron-seq-val", "poweron-seq-delay" }
};

static int parse_gpio_res(struct device *dev, struct device_node* node, dev_power_res_t* pwr_res)
{
	int gpio_no;
	int gpio_num;
	const char *gpio_name = NULL;
	int i = 0;
	int ret = 0;

	cam_info("Enter");

	gpio_num = of_property_count_strings(node, "gpio-names");
	if (gpio_num < 0) {
		cam_err("not fount gpio-names from dts");
		return -1;
	} else if (gpio_num > GPIO_NUM_MAX) {
		cam_warn("number of gpio resource exceeded the threshold[%d]", GPIO_NUM_MAX);
		gpio_num = GPIO_NUM_MAX;
	}
	pwr_res->num_of_gpio = gpio_num;
	cam_info("gpio_num = %d", gpio_num);
	for (i = 0; i < gpio_num; i++) {
		ret = of_property_read_string_index(node, "gpio-names", i, &gpio_name);
		if (ret) {
			cam_err("read gpio-names[%d]  fail %d", i, ret);
			return ret;
		}
		pwr_res->gpios[i].name = gpio_name;
		gpio_no = of_get_gpio(node, i);
		if (!gpio_is_valid(gpio_no)) {
			cam_err("%s is invalid", gpio_name);
			return -EINVAL;
		}
		pwr_res->gpios[i].gpio_no = gpio_no;
		ret = devm_gpio_request(dev, gpio_no, gpio_name);
		if (ret) {
			cam_err("request gpio[%d] name =%s  fail %d", gpio_no, gpio_name, ret);
			return ret;
		}
		ret = gpio_direction_output(gpio_no, 0);
		if (ret) {
			cam_err("set gpio[%d] name =%s direction output low  fail %d", gpio_no, gpio_name, ret);
			return ret;
		}
	}
	cam_info("Exit");
	return ret;
}

static int parse_ldo_res(struct device *dev, struct device_node* node, dev_power_res_t* pwr_res)
{
	int ldo_num;
	const char *ldo_name = NULL;
	int i;
	int ret = 0;

	cam_info("Enter");

	ldo_num = of_property_count_strings(node, "ldo-names");
	if (ldo_num < 0) {
		cam_err("not found ldo-names from dts");
		return -EINVAL;
	} else if (ldo_num > LDO_NUM_MAX) {
		cam_warn("number of ldo resource exceeded the threshold[%d]", LDO_NUM_MAX);
		ldo_num = LDO_NUM_MAX;
	}
	pwr_res->num_of_ldo = ldo_num;
	cam_info("ldo_num = %d", ldo_num);
	for (i = 0; i < ldo_num; i++) {
		ret = of_property_read_string_index(node, "ldo-names", i, &ldo_name);
		if (ret) {
			cam_err("read ldo-names[%d]  fail %d", i, ret);
			return ret;
		}
		pwr_res->ldos[i].name = ldo_name;

		pwr_res->ldos[i].supply = devm_regulator_get(dev, ldo_name);
		if (IS_ERR(pwr_res->ldos[i].supply)) {
			pwr_res->ldos[i].supply = NULL;
			cam_err("ldos[%d] devm_regulator_get fail", i);
			return -EINVAL;
		}
	}

	cam_info("Exit");
	return EOK;
}

static bool match_power_pin_res_info(power_pin_t *pin, dev_power_res_t* pwr_res)
{
	int i, num;
	const char *dts_suffix_name = NULL;
	const char *pwr_res_name = NULL;

	if (pin == NULL) {
		cam_err("power pin is NULL");
		return false;
	}

	if (strstr(pin->dts_name, "gpio_") != NULL) {
		pin->is_gpio = true;
		num = pwr_res->num_of_gpio;
		dts_suffix_name = pin->dts_name + strlen("gpio_");
	} else if (strstr(pin->dts_name, "ldo_") != NULL) {
		pin->is_gpio = false;
		num = pwr_res->num_of_ldo;
		dts_suffix_name = pin->dts_name + strlen("ldo_");
	} else {
		cam_err("unknown power pin. dts_name = %s\n", pin->dts_name);
		return false;
	}

	for (i = 0; i < num; i++) {
		pwr_res_name = pin->is_gpio ? pwr_res->gpios[i].name : pwr_res->ldos[i].name;
		if (strcmp(pwr_res_name, dts_suffix_name) != 0)
			continue;

		cam_info("power seq[%s] matched power resources[%s]\n", pin->dts_name, pwr_res_name);
		if (pin->is_gpio) {
			pin->gpio = &pwr_res->gpios[i];
		} else {
			pin->ldo = &pwr_res->ldos[i];
		}
		break;
	}
	if (i == num) {
		cam_err("power seq[%s] matched power resources fail \n", pin->dts_name);
		return false;
	}

	return true;
}


bool match_power_res(struct device_node* node, dev_power_res_t* pwr_res, dev_power_seq_t* pwr_seq,
							power_type_t power_type)
{
	int i;

	cam_info("Enter, power_type:%d.", power_type);

	if (POWER_TYPE_MAX <= power_type) {
		cam_err("power_type error, %d.", power_type);
		return false;
	}

	for (i = 0; i < pwr_seq->num; i++) {
		if (of_property_read_string_index(node, power_info_dts_string[power_type][POWER_DTS_SEQ_TYPE],
			i, &pwr_seq->pins[i].dts_name)) {
			cam_err("read power-seq-type[%d]  fail", i);
			return false;
		}

		if (of_property_read_u32_index(node, power_info_dts_string[power_type][POWER_DTS_SEQ_VAL],
			i, &pwr_seq->pins[i].val)) {
			cam_err("read power-seq-val[%d]  fail", i);
			return false;
		}

		if (of_property_read_u32_index(node, power_info_dts_string[power_type][POWER_DTS_SEQ_DELAY],
			i, &pwr_seq->pins[i].delay)) {
			cam_err("read power-seq-delay[%d]  fail", i);
			return false;
		}

		cam_info("match power pin[%d] resources.", i);
		if (match_power_pin_res_info(&pwr_seq->pins[i], pwr_res) != true) {
			cam_info("match power pin[%d] resources fail.", i);
			return false;
		}
	}

	return true;
}

static bool init_power_seq_data(struct device_node* node, dev_power_res_t* pwr_res, dev_power_seq_t* pwr_seq,
										power_type_t power_type)
{
	int num;
	bool val_exist;
	bool delay_exist;

	cam_info("Enter, power_type:%d.", power_type);

	if (POWER_TYPE_MAX <= power_type) {
		cam_err("power_type error, %d.", power_type);
		return false;
	}

	num = of_property_count_strings(node, power_info_dts_string[power_type][POWER_DTS_SEQ_TYPE]);
	cam_info("power-seq-type num = %d", num);
	if (num <= 0) {
		cam_err("not fount power-seq-type from dts");
		return false;
	} else if (num > DEV_POWER_PIN_MAX) {
		cam_warn("number of power-seq exceeded the threshold[%d]", DEV_POWER_PIN_MAX);
		num = DEV_POWER_PIN_MAX;
	}
	pwr_seq->num = num;

	val_exist = of_property_count_u32_elems(node, power_info_dts_string[power_type][POWER_DTS_SEQ_VAL]) == num;
	if (!val_exist) {
		cam_err("get power-seq-val fail");
		return false;
	}

	delay_exist = of_property_count_u32_elems(node, power_info_dts_string[power_type][POWER_DTS_SEQ_DELAY]) == num;
	if (!delay_exist) {
		cam_err("get power-seq-delay fail");
		return false;
	}

	return match_power_res(node, pwr_res, pwr_seq, power_type);
}

static int power_off(dev_power_seq_t *pwr_seq, external_device_type_t device_type)
{
	int i;
	int ret = 0;

	cam_info("pwr_seq->num = %d", pwr_seq->num);
	cam_info("device_type = %d", device_type);

	for (i = 0; i < pwr_seq->num; i++) {
		if (pwr_seq->pins[i].is_gpio) {
			if (pwr_seq->pins[i].gpio == NULL) {
				cam_err("power seq is NULL");
				return -1;
			}

			cam_info("gpio: %s, gpio_no:%d, val:%d, delay:%dms .",
					pwr_seq->pins[i].gpio->name,
					pwr_seq->pins[i].gpio->gpio_no,
					pwr_seq->pins[i].val,
					pwr_seq->pins[i].delay);
			gpio_set_value(pwr_seq->pins[i].gpio->gpio_no, pwr_seq->pins[i].val);
		} else {
			if (pwr_seq->pins[i].ldo == NULL || pwr_seq->pins[i].ldo->supply == NULL) {
				cam_err("power seq is NULL");
				return -1;
			}
			cam_info("regulator:%s, val:%dmv, delay:%dms .",
					pwr_seq->pins[i].ldo->name,
					pwr_seq->pins[i].val,
					pwr_seq->pins[i].delay);
			ret = regulator_set_voltage(pwr_seq->pins[i].ldo->supply, 0, INT_MAX);
			ret = regulator_disable(pwr_seq->pins[i].ldo->supply);
		}
		if (pwr_seq->pins[i].delay > DELAY_THRESHOLD) {
			usleep_range(pwr_seq->pins[i].delay * 1000, pwr_seq->pins[i].delay * 1000 + 1000);
		} else if (pwr_seq->pins[i].delay != 0) {
			mdelay(pwr_seq->pins[i].delay);
		}
	}

	return ret;
}

static void power_on_fail_deal(dev_power_seq_t *pwr_seq, int pin_index)
{
	for (pin_index = pin_index -1; pin_index >= 0; pin_index--) {
		if (pwr_seq->pins[pin_index].is_gpio) {
			cam_info("[power_up_fail]gpio:%s, gpio_no:%d, val:%d, delay:%dms.",
					pwr_seq->pins[pin_index].gpio->name,
					pwr_seq->pins[pin_index].gpio->gpio_no,
					!(pwr_seq->pins[pin_index].val),
					pwr_seq->pins[pin_index].delay);
			gpio_set_value(pwr_seq->pins[pin_index].gpio->gpio_no, !(pwr_seq->pins[pin_index].val));
		} else {
			cam_info("[power_up_fail]regulator:%s, val:%dmv, delay:%dms .",
					pwr_seq->pins[pin_index].ldo->name,
					pwr_seq->pins[pin_index].val,
					pwr_seq->pins[pin_index].delay);
			regulator_set_voltage(pwr_seq->pins[pin_index].ldo->supply, 0, INT_MAX);
			regulator_disable(pwr_seq->pins[pin_index].ldo->supply);
		}
		if (pwr_seq->pins[pin_index].delay > DELAY_THRESHOLD) {
			usleep_range(pwr_seq->pins[pin_index].delay * 1000, pwr_seq->pins[pin_index].delay * 1000 + 1000);
		} else if (pwr_seq->pins[pin_index].delay != 0) {
			mdelay(pwr_seq->pins[pin_index].delay);
		}
	}
}

static int power_on(dev_power_seq_t *pwr_seq, external_device_type_t device_type)
{
	int i;
	int ret = 0;

	cam_info("Enter pwr_seq.num:%d, device_type:%d .", pwr_seq->num, device_type);

	for (i = 0; i < pwr_seq->num; i++) {
		if (pwr_seq->pins[i].is_gpio) {
			if (pwr_seq->pins[i].gpio == NULL) {
				cam_err("gpio power seq[%d] is NULL", i);
				goto power_up_fail;
			}

			cam_info("gpio:%s, gpio_no:%d, val:%d, delay:%dms.",
					pwr_seq->pins[i].gpio->name,
					pwr_seq->pins[i].gpio->gpio_no,
					pwr_seq->pins[i].val,
					pwr_seq->pins[i].delay);
			gpio_set_value(pwr_seq->pins[i].gpio->gpio_no, pwr_seq->pins[i].val);
		} else {
			if (pwr_seq->pins[i].ldo == NULL) {
				cam_err("ldo power seq[%d] is NULL", i);
				goto power_up_fail;
			}
			cam_info("regulator:%s, val:%dmv, delay:%dms .",
					pwr_seq->pins[i].ldo->name,
					pwr_seq->pins[i].val,
					pwr_seq->pins[i].delay);
			ret = regulator_set_voltage(pwr_seq->pins[i].ldo->supply, pwr_seq->pins[i].val, pwr_seq->pins[i].val);
			ret = regulator_enable(pwr_seq->pins[i].ldo->supply);
		}
		if (pwr_seq->pins[i].delay > DELAY_THRESHOLD) {
			usleep_range(pwr_seq->pins[i].delay * 1000, pwr_seq->pins[i].delay * 1000 + 1000);
		} else if (pwr_seq->pins[i].delay != 0) {
			mdelay(pwr_seq->pins[i].delay);
		}
	}

	return ret;

power_up_fail:
	power_on_fail_deal(pwr_seq, i);

	return -1;
}

static int pcie2usb_dt_data_init(struct platform_device *pdev)
{
	int ret;

	cam_info("Enter");

	struct device_node* pcie2usb_node = of_find_node_by_name(g_camera->dev->of_node, "huawei,pcie2usb");
	of_property_read_u32(pcie2usb_node, "pcie_rc_idx", &g_camera->pcie2usb_data.pcie_rc_idx);
	if (g_camera->pcie2usb_data.pcie_rc_idx < 0) {
		cam_err("get pcie_rc_idx from dts fail");
		return -EINVAL;
	}

	if (parse_gpio_res(g_camera->dev, pcie2usb_node, &g_camera->pcie2usb_data.power_res) ||
		parse_ldo_res(g_camera->dev, pcie2usb_node, &g_camera->pcie2usb_data.power_res)) {
		cam_err("failed to parse pcie2usb power resources");
		return -EINVAL;
	}

	if (!init_power_seq_data(pcie2usb_node, &g_camera->pcie2usb_data.power_res,
							&g_camera->pcie2usb_data.poweron_seq, POWER_TYPE_ON)) {
		cam_err("init power power on seq form pcie2usb_node fail");
		return -EINVAL;
	}

	if (!init_power_seq_data(pcie2usb_node, &g_camera->pcie2usb_data.power_res,
							&g_camera->pcie2usb_data.poweroff_seq, POWER_TYPE_OFF)) {
		cam_err("get init power power off seq form pcie2usb_node fail");
		return -EINVAL;
	}

	of_property_read_u32(pcie2usb_node, "perst_pullhigh_index", &g_camera->pcie2usb_data.perst_pullhigh_index);
	if (g_camera->pcie2usb_data.perst_pullhigh_index < 0) {
		cam_err("get perst_pullhigh_index from dts fail");
		return -EINVAL;
	}

	g_camera->pcie2usb_data.power_status = POWER_STATUS_OFF;

	cam_info("Exit");
	return EOK;
}

static int isp_dt_data_init(struct platform_device *pdev)
{
	cam_info("Enter");
	struct device_node* isp_node = of_find_node_by_name(g_camera->dev->of_node, "huawei,external_isp");
	if (parse_gpio_res(g_camera->dev, isp_node, &g_camera->isp_data.power_res) ||
		parse_ldo_res(g_camera->dev, isp_node, &g_camera->isp_data.power_res)) {
		cam_err("failed to parse isp power resources");
		return -EINVAL;
	}

	if (!init_power_seq_data(isp_node, &g_camera->isp_data.power_res,
							&g_camera->isp_data.poweron_seq, POWER_TYPE_ON)) {
		cam_err("init power power on seq form isp_node fail");
		return -EINVAL;
	}

	if (!init_power_seq_data(isp_node, &g_camera->isp_data.power_res,
							&g_camera->isp_data.poweroff_seq, POWER_TYPE_OFF)) {
		cam_err("get init power power off seq form isp_node fail");
		return -EINVAL;
	}

	g_camera->isp_data.power_status = POWER_STATUS_OFF;

	cam_info("Exit");
	return EOK;
}

static int parse_sensor_res(struct device_node *sensor_node)
{
	int num_of_sensor = 0;
	struct device_node *node_temp = NULL;
	while (node_temp = of_get_next_child(sensor_node, node_temp))
		num_of_sensor++;

	if (num_of_sensor == 0) {
		cam_err(" No sensor found from dts");
		return -EINVAL;
	} else if (num_of_sensor > SENSOR_NUM_MAX) {
		cam_warn("number of sensor exceeded the threshold[%d]", SENSOR_NUM_MAX);
		num_of_sensor = SENSOR_NUM_MAX;
	}

	g_camera->sensor_data.num_of_sensor = num_of_sensor;
	g_all_sensor.num = num_of_sensor;
	cam_info("found %d camera sensor from dts", num_of_sensor);

	if (parse_gpio_res(g_camera->dev, sensor_node, &g_camera->sensor_data.power_res) ||
		parse_ldo_res(g_camera->dev, sensor_node, &g_camera->sensor_data.power_res)) {
		cam_err("failed to parse mipisw or sensor power resources");
		return -EINVAL;
	}

	return EOK;
}

static int sensor_dt_data_init(struct platform_device *pdev)
{
	int ret;
	int i = 0;
	struct device_node *sensor_node = NULL;
	struct device_node *node_temp = NULL;
	const char *name = NULL;

	sensor_node = of_find_node_by_name(g_camera->dev->of_node, "huawei,camera_sensor");
	if (sensor_node == NULL) {
		cam_err("sensor node is NULL");
		return -ENODEV;
	}
	ret = parse_sensor_res(sensor_node);
	if (ret < 0) {
		cam_err("parse_sensor_res fail:%x", ret);
		return ret;
	}

	for_each_child_of_node(sensor_node, node_temp) {
		ret = of_property_read_string(node_temp, "sensor_name", &name);
		if (ret < 0) {
			cam_err("get sensor_name fail");
			return ret;
		}
		memcpy_s(g_all_sensor.info[i].sensor_name, SENSOR_NAME_LEN_MAX, name, strlen(name));

		ret = of_property_read_u32(node_temp, "position", &g_all_sensor.info[i].position);
		if (ret < 0) {
			cam_err("get position fail");
			return ret;
		}

		ret = of_property_read_u32(node_temp, "id", &g_all_sensor.info[i].id);
		if (ret < 0) {
			cam_err("get id fail");
			return ret;
		}

		if (!init_power_seq_data(node_temp, &g_camera->sensor_data.power_res,
							&g_camera->sensor_data.poweron_seq[i], POWER_TYPE_ON)) {
			cam_err("init power power on seq form sensor_node fail");
			return -EINVAL;
		}

		if (!init_power_seq_data(node_temp, &g_camera->sensor_data.power_res,
							&g_camera->sensor_data.poweroff_seq[i], POWER_TYPE_OFF)) {
			cam_err("init power power off seq form sensor_node fail");
			return -EINVAL;
		}

		g_camera->sensor_data.power_status[i] = POWER_STATUS_OFF;

		i++;
	}

	g_camera->sensor_data.info = g_all_sensor.info;
	return EOK;
}

static int flash_dt_data_init(struct platform_device *pdev)
{
	int ret;

	cam_info("Enter");
	g_camera->flash_invalid = 0;
	ret = of_property_read_u32(g_camera->dev->of_node, "vendor,flash_invalid", &g_camera->flash_invalid);
	if (ret < 0) {
		cam_err("get flash_invalid fail");
		return ret;
	}

	cam_info("Exit");
	return EOK;
}

static int delay_poweroff_wl_init(struct platform_device *pdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	g_camera->delay_poweroff_wl = wakeup_source_register(&pdev->dev, "delay_poweroff_wl");
	if (!g_camera->delay_poweroff_wl)
		return -ENOMEM;
#else
	g_camera->delay_poweroff_wl = wakeup_source_register("delay_poweroff_wl");
	if (!g_camera->delay_poweroff_wl)
		return -ENOMEM;
#endif
	return EOK;
}

static int cfg_sensor(enum cfg_type type, int sensor_id)
{
	int ret = -1;

	if (sensor_id >= 0 && sensor_id < g_camera->sensor_data.num_of_sensor) {
		cam_info("power config for %s, id = %d ",
		g_camera->sensor_data.info[sensor_id].sensor_name, sensor_id);
	} else {
		cam_err("invalid id %d", sensor_id);
		return -EINVAL;
	}

	switch (type) {
	case SENSOR_POWER_UP:
		if (g_camera->sensor_data.power_status[sensor_id] == POWER_STATUS_OFF) {
			ret = power_on(&g_camera->sensor_data.poweron_seq[sensor_id], EXTERNAL_DEVICE_TYPE_SENSOR);
			if (ret) {
				cam_err("sensor power_on fail");
			} else {
				g_camera->sensor_data.power_status[sensor_id] = POWER_STATUS_ON;
			}
		} else {
			cam_info("SENSOR_POWER_UP already, sensor_data[%d] status:%d.",
					sensor_id, g_camera->sensor_data.power_status[sensor_id]);
			ret = 0;
		}
		break;
	case SENSOR_POWER_DOWN:
		if (g_camera->sensor_data.power_status[sensor_id] == POWER_STATUS_ON) {
			ret = power_off(&g_camera->sensor_data.poweroff_seq[sensor_id], EXTERNAL_DEVICE_TYPE_SENSOR);
			if (ret) {
				cam_err("sensor power_off fail");
			} else {
				g_camera->sensor_data.power_status[sensor_id] = POWER_STATUS_OFF;
			}
		} else {
			cam_info("SENSOR_POWER_DOWN already, sensor_data[%d] status:%d.",
					sensor_id, g_camera->sensor_data.power_status[sensor_id]);
			ret = 0;
		}
		break;
	default:
		cam_err("unknown sensor ioctl cfg type");
		break;
	}

	return ret;
}

static int cfg_pcie2usb_and_isp(dev_power_seq_t *pwr_seq, power_status_t *cur_pwr_status,
	power_status_t cfg_pwr_status, external_device_type_t device_type)
{
	int ret = 0;

	if (pwr_seq == NULL || cur_pwr_status == NULL) {
		cam_err("pwr_seq or cur_pwr_status  is NULL");
		return -1;
	}

	cam_info("cur power state[%d], cfg power state[%d].", *cur_pwr_status, cfg_pwr_status);
	if ((*cur_pwr_status) == cfg_pwr_status) {
		cam_info("No change in status.");
		return 0;
	}

	if ((*cur_pwr_status) == POWER_STATUS_OFF) {
		ret = power_on(pwr_seq, device_type);
		if (ret) {
			cam_err("power on device_type[%d] fail", device_type);
		} else {
			(*cur_pwr_status) = POWER_STATUS_ON;
			if (device_type == EXTERNAL_DEVICE_TYPE_PCIE2USB &&
				g_camera->pcie2usb_data.perst_pullhigh_index > 0 &&
				g_camera->pcie2usb_data.perst_pullhigh_index < g_camera->pcie2usb_data.poweron_seq.num)
				pwr_seq->pins[g_camera->pcie2usb_data.perst_pullhigh_index - 1].delay = 5;
			cam_info("power on device_type[%d] success", device_type);
		}
	} else if ((*cur_pwr_status) == POWER_STATUS_ON) {
		ret = power_off(pwr_seq, device_type);
		if (ret) {
			cam_err("power off device_type[%d] fail", device_type);
		} else {
			(*cur_pwr_status) = POWER_STATUS_OFF;
			cam_info("power off device_type[%d] success", device_type);
		}
	} else {
		cam_err("error cur power status[%d].", *cur_pwr_status);
		ret = -1;
	}

	return ret;
}

static int cfg_ioctl_handler(unsigned long arg)
{
	int ret = -1;
	struct cfg_data data;
	void __user *argp = (void __user *)arg;
	if (argp == NULL) {
		cam_err("argp is NULL!");
		return ret;
	}
	cam_info("Enter");
	if (copy_from_user(&data, argp, sizeof(data))) {
		cam_err("copy_from_user error!");
		return ret;
	}

	cam_info("ioctl cmd type: %u", data.type);

	mutex_lock(&g_mutex_power);
	switch (data.type) {
	case PCIE2USB_POWER_UP:
		ret = 0;

		break;
	case PCIE2USB_POWER_DOWN:
		ret = 0;
		break;
	case ISP_POWER_UP:
		ret = cfg_pcie2usb_and_isp(&g_camera->isp_data.poweron_seq, &g_camera->isp_data.power_status,
			POWER_STATUS_ON, EXTERNAL_DEVICE_TYPE_ISP);
		break;
	case ISP_POWER_DOWN:
		ret = cfg_pcie2usb_and_isp(&g_camera->isp_data.poweroff_seq, &g_camera->isp_data.power_status,
			POWER_STATUS_OFF, EXTERNAL_DEVICE_TYPE_ISP);
		break;
	case PCIE2USB_ENUMERATE_DEVICE:
		ret = pcie_kport_enumerate(g_camera->pcie2usb_data.pcie_rc_idx);
		break;
	case SENSOR_POWER_UP:
	case SENSOR_POWER_DOWN:
		ret = cfg_sensor(data.type, data.parameter);
		break;
	default:
		cam_err("unknown ioctl cfg type");
		break;
	}
	mutex_unlock(&g_mutex_power);

	cam_info("Exit");
	return ret;
}

static int get_sensor_info_ioctl_handler(unsigned long arg)
{
	cam_info("Enter");
	void __user *argp = (void __user *)arg;
	if (argp == NULL) {
		cam_err("argp is NULL!");
		return -EINVAL;
	}

	if (copy_to_user((void __user *)arg, &g_all_sensor, sizeof(sensor_list_t))) {
		cam_err("failed to copy to user");
		return -EINVAL;
	}
	cam_info("Exit");
	return EOK;
}

static int get_flash_invalid_ioctl_handler(unsigned long arg)
{
	cam_info("Enter");
	void __user *argp = (void __user *)arg;
	if (argp == NULL) {
		cam_err("argp is NULL!");
		return -EINVAL;
	}

	if (copy_to_user((void __user *)arg, &g_camera->flash_invalid, sizeof(int))) {
		cam_err("failed to copy to user");
		return -EINVAL;
	}
	cam_info("Exit");
	return EOK;
}

#ifdef CONFIG_DKMD_DKSM
static int g_screentype = 0;
static int get_screenstatus_ioctl_handler(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	if (argp == NULL) {
		cam_err("argp is NULL!");
		return -EINVAL;
	}

	if (copy_to_user((void __user *)arg, &g_screentype, sizeof(int))) {
		cam_err("screenstatufailed to copy to user");
		return -EINVAL;
	}
	return EOK;
}

static int external_cam_fb_notifier_call(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct dkmd_event *event = NULL;

	if (!data || !nb)
		return NOTIFY_OK;

	if (action != DKMD_EVENT_BLANK)
		return NOTIFY_OK;

	event = (struct dkmd_event *)data;
	g_screentype = event->value;
	cam_info(" fb blank=%d\n", g_screentype);

	return NOTIFY_OK;
}

static struct notifier_block external_cam_fb_notifier_block = {
	.notifier_call = external_cam_fb_notifier_call,
};
#endif

static int cfg_delay_poweroff_wl(unsigned long arg)
{
	int ret = -1;
	int delay_val = 0;
	void __user *argp = (void __user *)arg;
	if (argp == NULL) {
		cam_err("argp is NULL!");
		return ret;
	}
	cam_info("Enter");
	if (copy_from_user(&delay_val, argp, sizeof(delay_val))) {
		cam_err("copy_from_user error!");
		return ret;
	}

	if (delay_val >= MAX_DELAY_POWEROFF_WL_VAL) {
		delay_val = MAX_DELAY_POWEROFF_WL_VAL;
	} else if (delay_val <= MIN_DELAY_POWEROFF_WL_VAL) {
		delay_val = MIN_DELAY_POWEROFF_WL_VAL;
	}

	cam_info("delay poweroff wl val: %d", delay_val);

	if (g_camera->isp_data.power_status == POWER_STATUS_ON)
			__pm_wakeup_event(g_camera->delay_poweroff_wl, delay_val);

	cam_info("Exit");
	return EOK;
}

static int pcie_device_power_on(void *data)
{
	int ret = -1;

	ret = cfg_pcie2usb_and_isp(&g_camera->pcie2usb_data.poweron_seq, &g_camera->pcie2usb_data.power_status,
		POWER_STATUS_ON, EXTERNAL_DEVICE_TYPE_PCIE2USB);
	cam_info("pcie power_on return ret:%d", ret);

	return ret;
}

static int pcie_device_power_off(void *data)
{
	int ret = -1;

	ret = cfg_pcie2usb_and_isp(&g_camera->pcie2usb_data.poweroff_seq, &g_camera->pcie2usb_data.power_status,
		POWER_STATUS_OFF, EXTERNAL_DEVICE_TYPE_PCIE2USB);
	cam_info("pcie power_off return ret:%d", ret);

	return ret;
}

static long external_cam_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	int ret = -1;

	cam_info("ioctl cmd: %u", cmd);
	switch (cmd) {
	case EXCAM_IOCTL_CFG:
		ret = cfg_ioctl_handler(arg);
		break;
	case EXCAM_IOCTL_GET_INFO:
		ret = get_sensor_info_ioctl_handler(arg);
		break;
	case EXCAM_IOCTL_GET_FLASH_INVALID:
		ret = get_flash_invalid_ioctl_handler(arg);
		break;
#ifdef CONFIG_DKMD_DKSM
	case EXCAM_IOCTL_GET_SCREENSTATUS:
		ret = get_screenstatus_ioctl_handler(arg);
		break;
#endif
	case EXCAM_IOCTL_CFG_DELAY_POWEROFF:
		ret = cfg_delay_poweroff_wl(arg);
		break;
	default:
		cam_err("unknown cmd!");
		break;
	}
	return ret;
}

static int external_cam_dev_open(struct inode *inode, struct file *filp)
{
	cam_info("Enter");
	int ret = -1;

	ret = pcie_kport_power_notifiy_register(g_camera->pcie2usb_data.pcie_rc_idx,
		pcie_device_power_on, pcie_device_power_off, NULL);
	cam_info("pcie_kport_power_notifiy_register ret:%d", ret);

	return EOK;
}

static int external_cam_dev_release(struct inode *inode, struct file *filp)
{
	cam_info("Enter");

	int ret = 0;
	mutex_lock(&g_mutex_power);
	for (int i = 0; i < g_camera->sensor_data.num_of_sensor; i++) {
		if (g_camera->sensor_data.power_status[i] == POWER_STATUS_ON) {
			if (ret = power_off(&g_camera->sensor_data.poweroff_seq[i], EXTERNAL_DEVICE_TYPE_SENSOR)) {
				cam_err("sensor power_off fail");
			} else {
				g_camera->sensor_data.power_status[i] = POWER_STATUS_OFF;
			}
		}
	}

	if (g_camera->isp_data.power_status == POWER_STATUS_ON) {
		cfg_pcie2usb_and_isp(&g_camera->isp_data.poweroff_seq, &g_camera->isp_data.power_status,
			POWER_STATUS_OFF, EXTERNAL_DEVICE_TYPE_ISP);
	}

	if (g_camera->delay_poweroff_wl->active)
		__pm_relax(g_camera->delay_poweroff_wl);

	mutex_unlock(&g_mutex_power);

	return EOK;
}

static struct file_operations external_cam_dev_fops = {
	.owner = THIS_MODULE,
	.open = external_cam_dev_open,
	.release = external_cam_dev_release,
	.unlocked_ioctl = external_cam_ioctl,
};

static int external_camera_get_dts_init(struct platform_device *pdev)
{
	int ret = -1;

	ret = pcie2usb_dt_data_init(pdev);
	if (ret < 0) {
		cam_err("pcie2usb dt data init fail");
		return -EINVAL;
	}

	ret = isp_dt_data_init(pdev);
	if (ret < 0) {
		cam_err("isp dt data init fail");
		return -EINVAL;
	}

	ret = sensor_dt_data_init(pdev);
	if (ret < 0) {
		cam_err("sensor dt data init fail");
		return -EINVAL;
	}

	ret = flash_dt_data_init(pdev);
	if (ret < 0) {
		cam_err("flash dt data init fail");
		return -EINVAL;
	}

	ret = delay_poweroff_wl_init(pdev);
	if (ret < 0) {
		cam_err("delay poweroff wl init fail");
		return -EINVAL;
	}

	return EOK;
}

static int external_camera_dev_init(struct platform_device *pdev)
{
	int ret = -1;

	g_camera = devm_kzalloc(&pdev->dev, sizeof(external_camera_data_t), GFP_KERNEL);
	if (!g_camera) {
		cam_err("fail to kzalloc!");
		goto alloc_err;
	}
	g_camera->dev = &pdev->dev;

	ret = alloc_chrdev_region(&g_excam_device.cam_devno, 0, EXCAM_DEV_CNT, EXCAM_DEV_NAME);
	if (ret < 0) {
		cam_err("fail to alloc cam_devno");
		goto alloc_chrdev_err;
	}

	g_excam_device.excam_dev.owner = THIS_MODULE;
	cdev_init(&g_excam_device.excam_dev, &external_cam_dev_fops);

	ret = cdev_add(&g_excam_device.excam_dev, g_excam_device.cam_devno, EXCAM_DEV_CNT);
	if (ret < 0) {
		cam_err("fail to add cdev");
		goto add_err;
	}

	g_excam_device.class_cam = class_create(THIS_MODULE, EXCAM_DEV_NAME);
	if (IS_ERR(g_excam_device.class_cam)) {
		cam_err("creat class fail(%ld)!\n", PTR_ERR(g_excam_device.class_cam));
		goto class_create_err;
	}
	g_excam_device.device = device_create(g_excam_device.class_cam, NULL, g_excam_device.cam_devno, NULL, EXCAM_DEV_NAME);
	if (IS_ERR(g_excam_device.device)) {
		cam_err("creat dev fail(%ld)!\n", PTR_ERR(g_excam_device.device));
		goto dev_create_err;
	}

	ret = external_camera_get_dts_init(pdev);
	if (ret < 0) {
		cam_err("get dts init fail");
		goto get_dts_init_err;
	}

	return EOK;

get_dts_init_err:
	device_destroy(g_excam_device.class_cam, g_excam_device.cam_devno);
dev_create_err:
	class_destroy(g_excam_device.class_cam);
class_create_err:
	cdev_del(&g_excam_device.excam_dev);
add_err:
	unregister_chrdev_region(g_excam_device.cam_devno, EXCAM_DEV_CNT);
alloc_chrdev_err:
	devm_kfree(&pdev->dev, g_camera);
	g_camera = NULL;
alloc_err:
	return -1;
}

static int external_camera_probe(struct platform_device *pdev)
{
	int ret = -1;

	ret = external_camera_dev_init(pdev);
	if (ret < 0) {
		cam_err("external camera dev init fail");
		return -EINVAL;
	}

#ifdef CONFIG_DKMD_DKSM
	dkmd_register_client(&external_cam_fb_notifier_block);
#endif

	mutex_init(&g_mutex_power);

	return EOK;
}

static int external_camera_remove(struct platform_device *pdev)
{
	cam_info("Enter");

	wakeup_source_unregister(g_camera->delay_poweroff_wl);
	g_camera->delay_poweroff_wl = NULL;

	devm_kfree(&pdev->dev, g_camera);
	g_camera = NULL;

	device_destroy(g_excam_device.class_cam, g_excam_device.cam_devno);
	class_destroy(g_excam_device.class_cam);
	cdev_del(&g_excam_device.excam_dev);
	unregister_chrdev_region(g_excam_device.cam_devno, EXCAM_DEV_CNT);

#ifdef CONFIG_DKMD_DKSM
	dkmd_unregister_client(&external_cam_fb_notifier_block);
#endif

	cam_info("Exit");
	return EOK;
}

static int external_camera_suspend(struct device *dev)
{
	cam_info("Enter");

	if (g_camera->isp_data.power_status == POWER_STATUS_ON) {
		cfg_pcie2usb_and_isp(&g_camera->isp_data.poweroff_seq, &g_camera->isp_data.power_status,
			POWER_STATUS_OFF, EXTERNAL_DEVICE_TYPE_ISP);
	}

	cam_info("Exit");

	return 0;
}

static int external_camera_resume(struct device *dev)
{
	cam_info("Enter");

	cam_info("pcie_kport_lp_ctrl enable, return:%d.",
		pcie_kport_lp_ctrl(g_camera->pcie2usb_data.pcie_rc_idx, 1));

	cam_info("Exit");

	return 0;
}

static const struct dev_pm_ops external_camera_pm_ops = {
	.suspend	= external_camera_suspend,
	.resume		= external_camera_resume,
};

static const struct of_device_id external_camera_match_table[] = {
	{ .compatible = "huawei,external_camera" },
	{},
};

static struct platform_driver external_camera_driver = {
	.probe = external_camera_probe,
	.remove = external_camera_remove,
	.driver = {
		.name = "external_camera",
		.pm	= &external_camera_pm_ops,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(external_camera_match_table),
	},
};

static int __init external_camera_init(void)
{
	cam_info("Enter");
	return platform_driver_register(&external_camera_driver);
}

static void __exit external_camera_exit(void)
{
	platform_driver_unregister(&external_camera_driver);
}

module_init(external_camera_init);
module_exit(external_camera_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei external camera control module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
